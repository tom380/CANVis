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
#include <map>

#include "implot.h"

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define UNSIGNED 0
#define SIGNED 1

namespace CAN {
    struct SignalDescription;
    struct MessageDescription;

    struct Message;
    class MessageBuffer;

    void parseDBC(const std::string& filename, std::map<int, CAN::MessageDescription>& dbc);
}

struct CAN::SignalDescription {
    std::string name;
    int startBit;
    size_t length;
    bool endianess;
    bool signedness;
    float scale;
    float offset;
    float min;
    float max;
    std::string unit;
};

struct CAN::MessageDescription {
    unsigned long id;
    std::string name;
    size_t length;
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
    explicit MessageBuffer(size_t maxSize);

    void addMessage(const Message& message);
    void setMaxSize(size_t maxSize);

    const std::deque<Message*>& ofID(int id) const;
    // ImPlotPoint getter<int>(int idx, void* label);

    std::deque<Message>::iterator begin();
    std::deque<Message>::iterator end();
    std::deque<Message>::const_iterator begin() const;
    std::deque<Message>::const_iterator end() const;
};