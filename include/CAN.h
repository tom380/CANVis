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
    size_t currentSize = 0; // Current size in bytes
    size_t maxSize;         // Maximum allowed size in bytes
    std::mutex bufferMutex; // To protect the buffer in multi-threaded environments

    // Helper to calculate the size of a single message
    size_t calculateMessageSize(const Message& message) const {
        return sizeof(message.id) + message.rawData.size();
    }

    // Enforce max size by removing oldest messages
    void enforceMaxSize() {
        while (currentSize > maxSize && !messages.empty()) {
            const auto& oldest = messages.front();
            currentSize -= calculateMessageSize(oldest);
            messages.pop_front();
        }
    }

public:
    // Constructor
    explicit MessageBuffer(size_t maxSizeInBytes)
        : maxSize(maxSizeInBytes) {}

    // Add a new message
    void addMessage(const Message& message) {
        std::lock_guard<std::mutex> lock(bufferMutex);
        size_t messageSize = calculateMessageSize(message);
        
        if (messageSize > maxSize) {
            throw std::runtime_error("Message size exceeds buffer's maximum size.");
        }

        messages.push_back(message);
        currentSize += messageSize;

        enforceMaxSize();
    }

    // Get all stored messages
    std::vector<Message> getMessages() {
        std::lock_guard<std::mutex> lock(bufferMutex);
        return std::vector<Message>(messages.begin(), messages.end());
    }

    // Clear all messages
    void clear() {
        std::lock_guard<std::mutex> lock(bufferMutex);
        messages.clear();
        currentSize = 0;
    }

    // Set a new max size
    void setMaxSize(size_t newMaxSize) {
        std::lock_guard<std::mutex> lock(bufferMutex);
        maxSize = newMaxSize;
        enforceMaxSize();
    }

    // Get the current size in bytes
    size_t getCurrentSize() {
        std::lock_guard<std::mutex> lock(bufferMutex);
        return currentSize;
    }
};