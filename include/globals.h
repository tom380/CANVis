#pragma once

#include <map>
#include <deque>
#include <string>
#include "CAN.h"

inline long handle = NULL;

inline const int baudrates[] = {20, 50, 100, 125, 250, 500, 800, 1000};
inline int baudrate = 500;

inline std::map<int, CAN::MessageDescription> messageDescriptions;
inline CAN::MessageBuffer messageBuffer(500);

typedef std::vector<std::pair<unsigned long, float>> Plot;
inline std::vector<Plot> plots;

inline bool isPaused = true;