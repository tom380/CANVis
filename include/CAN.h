#pragma once

#include <string>
#include <deque>
#include <vector>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include "usb2can.h"
#include <any>
#include <unordered_map>

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define UNSIGNED 0
#define SIGNED 1

namespace CAN {
    struct SignalDescription;
    struct MessageDescription;

    struct Message;
    class MessageBuffer;
}

struct CAN::SignalDescription {
    std::string name;
    int startBit;
    int length;
    bool endianess;
    bool signedness;
    float scale;
    float offset;
    float min;
    float max;
    std::string unit;
};

struct CAN::MessageDescription {
    int id;
    std::string name;
    int length;
    std::string sender;
    std::vector<SignalDescription> signals;

    bool plot = false;
};

struct CAN::Message {
    unsigned long flags;
    unsigned long obid;
    unsigned long id;
    unsigned char sizeData;
    std::vector<uint8_t> rawData;
    std::unordered_map<std::string, std::any> decodedData;
    unsigned long timestamp;

    Message(const CANALMSG& canalMessage);

    
    void decode();

    template <typename T>
    T getDecodedValue(const std::string& name) const {
        auto it = decodedData.find(name);
        if (it == decodedData.end()) {
            throw std::runtime_error("Variable not found: " + name);
        }
        return std::any_cast<T>(it->second);
    }
    
private:
    std::any extractSignal(const SignalDescription& signalDescription);
};

class CAN::MessageBuffer {
private:
    std::deque<Message> messages;
    std::unordered_map<int, std::deque<Message*>> messageMap;
    size_t maxSize;

    void pop_front();

public:
    MessageBuffer(size_t maxSize);

    void addMessage(const Message& message);
    void setMaxSize(size_t maxSize);

    std::deque<CAN::Message>::iterator begin();
    std::deque<CAN::Message>::iterator end();
    std::deque<CAN::Message>::const_iterator begin() const;
    std::deque<CAN::Message>::const_iterator end() const;
};