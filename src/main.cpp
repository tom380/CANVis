#include "globals.h"
#include "Window.h"

#include <iostream>
#include "usb2can.h"
#include <vector>
#include "CAN.h"

int main() {
    std::vector<CAN::SignalDescription> signals;
    signals.push_back(CAN::SignalDescription{"AccX", 7, 16, BIG_ENDIAN, SIGNED, 0.00390625, 0, -100, 100, "m/s²"});
    signals.push_back(CAN::SignalDescription{"AccY", 23, 16, BIG_ENDIAN, SIGNED, 0.00390625, 0, -100, 100, "m/s²"});
    signals.push_back(CAN::SignalDescription{"AccZ", 39, 16, BIG_ENDIAN, SIGNED, 0.00390625, 0, -100, 100, "m/s²"});
    CAN::MessageDescription mDes = {0x34, "Acceleration", 6, "Xsens_Sensor", signals};
    messageDescriptions.push_back(mDes);

    mDes.id = 0x35;
    mDes.name = "FreeAcceleration";
    mDes.signals[0].name = "FreeAccX";
    mDes.signals[1].name = "FreeAccY";
    mDes.signals[2].name = "FreeAccZ";
    messageDescriptions.push_back(mDes);
    mDes.id = 0x36;
    messageDescriptions.push_back(mDes);
    mDes.id = 0x37;
    messageDescriptions.push_back(mDes);

    Window window(1280, 720, "CANVis");

    while (!window.exit()) {
        if (handle > 0) {
            CANALMSG msg;

            while (CanalDataAvailable(handle)) {
                CanalReceive(handle, &msg);
                float accx = int16_t((msg.data[1] << 8) | msg.data[0]) / (float)256;
                float accy = int16_t((msg.data[3] << 8) | msg.data[2]) / (float)256;
                float accz = int16_t((msg.data[5] << 8) | msg.data[4]) / (float)256;

                if (tData.size() >= 100) {
                    tData.erase(tData.begin());
                    xData.erase(xData.begin());
                    yData.erase(yData.begin());
                    zData.erase(zData.begin());
                }
                
                tData.push_back(tData.size() == 0 ? 0 : tData.back() + 1);
                xData.push_back(accx);
                yData.push_back(accy);
                zData.push_back(accz);
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