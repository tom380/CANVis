#include "CAN.h"

#include "globals.h"

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