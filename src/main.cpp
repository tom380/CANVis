#include "globals.h"
#include "Window.h"

#include <iostream>
#include "usb2can.h"
#include <vector>
#include "CAN.h"

int main() {
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