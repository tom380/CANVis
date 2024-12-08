#include "globals.h"
#include "Window.h"

#include <iostream>
#include "usb2can.h"
#include <vector>

int main() {
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