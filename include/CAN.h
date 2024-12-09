#pragma once

#include <string>
#include <vector>

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define UNSIGNED 0
#define SIGNED 1

namespace CAN {
    struct SignalDescription;
    struct MessageDescription;
}

struct CAN::SignalDescription {
    std::string name;
    int start;
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