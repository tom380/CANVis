#include "CAN.h"

#include "globals.h"

#include <fstream>
#include <sstream>
#include <iostream>

CAN::Message::Message(const CANALMSG& canalMessage) : flags(canalMessage.flags),
                                                      obid(canalMessage.obid),
                                                      id(canalMessage.id),
                                                      sizeData(canalMessage.sizeData),
                                                      timestamp(canalMessage.timestamp) {
    for (int i = 0; i < sizeData; i++) {
        rawData.push_back(canalMessage.data[i]);
    }
    
    // TODO: Use catch with specific error instead of duplicate checking
    if (messageDescriptions.find(id) != messageDescriptions.end()) decode();
}

void CAN::Message::decode() {
    auto it = messageDescriptions.find(id);
    if (it == messageDescriptions.end()) throw std::runtime_error("No message description found for this message");

    MessageDescription& description = it->second;
        

    for (const CAN::SignalDescription& sigDes : description.signals) {
        decodedData[sigDes.name] = extractSignal(sigDes);
    }
}

std::any CAN::Message::extractSignal(const SignalDescription& sigDes) {
    int byteStart = sigDes.startBit / 8;
    int bitStart = 7 - (sigDes.startBit % 8);
    int bitEnd = (sigDes.startBit + sigDes.length - 1) % 8;

    // Extract bits
    uint64_t value = 0;
    const int nBytes = (sigDes.length + 7) / 8;
    for (int i = 0; i < nBytes; ++i) {
        int byteIndex;
        switch (sigDes.endianess) {
            case LITTLE_ENDIAN: byteIndex = byteStart + (nBytes - 1 - i); break;
            case BIG_ENDIAN: byteIndex = byteStart + i; break;
        }
        value = (value << 8) | rawData[byteIndex];
    }

    // Mask out bits not in range
    value >>= (8 * nBytes - sigDes.length - bitStart);
    value &= (1ULL << sigDes.length) - 1;

    // Handle signedness
    int64_t signedValue = static_cast<int64_t>(value);
    if (sigDes.signedness && (value & (1ULL << (sigDes.length - 1)))) {
        signedValue -= (1ULL << sigDes.length);
    }

    // Apply scale and offset
    double result = static_cast<double>(signedValue) * sigDes.scale + sigDes.offset;

    // Return as appropriate type
    if (sigDes.scale == 1.0 && sigDes.offset == 0.0) {
        return signedValue; // Treat as integer
    }
    return result; // Treat as float
}

CAN::MessageBuffer::MessageBuffer(size_t maxSize) : maxSize(maxSize) {

}

void CAN::MessageBuffer::pop_front() {
        messageMap[messages.front().id].pop_front();
        messages.pop_front();
}

void CAN::MessageBuffer::addMessage(const Message& message) {
    if (messages.size() == maxSize) pop_front();

    messages.push_back(message);
    messageMap[message.id].push_back(&messages.back());
}

void CAN::MessageBuffer::setMaxSize(size_t maxSize) {
    while (messages.size() > maxSize) pop_front();

    this->maxSize = maxSize;
}

const std::deque<CAN::Message*>& CAN::MessageBuffer::ofID(int id) const {
    static const std::deque<Message*> emptyQueue;
    auto it = messageMap.find(id);
    return it != messageMap.end() ? it->second : emptyQueue;
}

std::deque<CAN::Message>::iterator CAN::MessageBuffer::begin() {
    return messages.begin();
}

std::deque<CAN::Message>::iterator CAN::MessageBuffer::end() {
    return messages.end();
}

std::deque<CAN::Message>::const_iterator CAN::MessageBuffer::begin() const {
    return messages.cbegin();
}

std::deque<CAN::Message>::const_iterator CAN::MessageBuffer::end() const {
    return messages.cend();
}

void CAN::parseDBC(const std::string& filename, std::map<int, CAN::MessageDescription>& dbc) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    int lastMsgID = -1;
    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string token;
        stream >> token;

        if (token == "BU_:") {
            // Node
        } else if (token == "BO_") {
            // Message
            CAN::MessageDescription msg;
            stream >> msg.id >> msg.name;
            msg.name.pop_back(); // Remove trailing ':'
            stream >> msg.length >> msg.sender;
            dbc[msg.id] = msg;
            lastMsgID = msg.id;
        } else if (token == "SG_") {
            // Parse Signal
            CAN::SignalDescription signal;
            CAN::MessageDescription& msg = dbc[lastMsgID]; // Last added message

            std::string colon;
            stream >> signal.name >> colon;
            
            std::string bitInfo, scalingInfo, rangeInfo, unit, receiver;
            stream >> bitInfo >> scalingInfo >> rangeInfo >> unit >> receiver;

            // Parse bit start and length (e.g., "0|16@1+")
            size_t pipePos = bitInfo.find('|');
            size_t atPos = bitInfo.find('@');
            signal.startBit = std::stoi(bitInfo.substr(0, pipePos));
            signal.length = std::stoi(bitInfo.substr(pipePos + 1, atPos - pipePos - 1));
            signal.endianess = bitInfo[atPos + 1] == '1';
            signal.signedness = bitInfo[atPos + 2] == '-';

            // Parse scaling and offset (e.g., "(0.1,0)")
            size_t commaPos = scalingInfo.find(',');
            signal.scale = std::stof(scalingInfo.substr(1, commaPos - 1)); // Remove '('
            signal.offset = std::stof(scalingInfo.substr(commaPos + 1, scalingInfo.size() - commaPos - 2)); // Remove ')'

            // Parse range (e.g., "[0|10000]")
            size_t bracketPos = rangeInfo.find('|');
            signal.min = std::stof(rangeInfo.substr(1, bracketPos - 1)); // Remove '['
            signal.max = std::stof(rangeInfo.substr(bracketPos + 1, rangeInfo.size() - bracketPos - 2)); // Remove ']'

            // Parse unit and receiver
            signal.unit = unit.substr(1, unit.size() - 2); // Remove quotes
            // signal.receiver = receiver;

            msg.signals.push_back(signal);
        }
    }

    file.close();
}