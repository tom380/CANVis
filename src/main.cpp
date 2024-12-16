#include "globals.h"
#include "Window.h"

#include <iostream>
#include "usb2can.h"
#include <vector>
#include "CAN.h"

int main() {
    std::vector<CAN::SignalDescription> signals;
    signals.push_back(CAN::SignalDescription{"AccX", 7, 16, LITTLE_ENDIAN, SIGNED, 0.00390625, 0, -100, 100, "m/s²"});
    signals.push_back(CAN::SignalDescription{"AccY", 23, 16, LITTLE_ENDIAN, SIGNED, 0.00390625, 0, -100, 100, "m/s²"});
    signals.push_back(CAN::SignalDescription{"AccZ", 39, 16, LITTLE_ENDIAN, SIGNED, 0.00390625, 0, -100, 100, "m/s²"});
    CAN::MessageDescription mDes = {0x34, "Acceleration", 6, "Xsens_Sensor", signals};
    messageDescriptions[mDes.id] = mDes;

    mDes.id = 0x35;
    mDes.name = "FreeAcceleration";
    mDes.signals[0].name = "FreeAccX";
    mDes.signals[1].name = "FreeAccY";
    mDes.signals[2].name = "FreeAccZ";
    messageDescriptions[mDes.id] = mDes;
    mDes.id = 0x36;
    messageDescriptions[mDes.id] = mDes;
    mDes.id = 0x37;
    messageDescriptions[mDes.id] = mDes;

    Window window(1280, 720, "CANVis");

    while (!window.exit()) {
        if (handle > 0) {
            CANALMSG msg;

            while (CanalDataAvailable(handle)) {
                CanalReceive(handle, &msg);
                if (!isPaused) messageBuffer.addMessage(msg);
            }
        }

        window.update();
    }

    window.close();

    if (CanalClose(handle) != 0) {
        std::cerr << "Failed to close the CAN channel!" << std::endl;
    }

    return 0;
}