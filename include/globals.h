#pragma once

#include <unordered_map>
#include <deque>
#include <string>
#include "CAN.h"

inline std::vector<float> tData;
inline std::vector<float> xData;
inline std::vector<float> yData;
inline std::vector<float> zData;

inline long handle = NULL;

inline const int baudrates[] = {20, 50, 100, 125, 250, 500, 800, 1000};
inline int baudrate = 500;

inline std::unordered_map<int, CAN::MessageDescription> messageDescriptions;
inline CAN::MessageBuffer messageBuffer(5000);

typedef std::vector<std::pair<unsigned long, float>> Plot;
inline std::vector<Plot> plots;

inline bool isPaused = true;