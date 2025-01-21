#pragma once

#include <GLFW/glfw3.h>
#include <string>

class Window {
private:
    GLFWwindow* pWindow;
    int width, height;

    void createImGui();
    void createSettingsTab();
    void createDatabaseTab();
    void createTransmitTab();
    void createMonitorTab();
    void createGraphTab();

    std::string openFileDialog();

public:
    Window(int width, int height, const char* title);
    ~Window();

    void update();
    void close();
    bool exit();
};