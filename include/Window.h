#pragma once

#include <GLFW/glfw3.h>

class Window {
private:
    GLFWwindow* pWindow;
    int width, height;

    void createImGui();
    void createSettingsTab();
    void createDatabaseTab();
    void createMonitorTab();
    void createGraphTab();

public:
    Window(int width, int height, const char* title);
    ~Window();

    void update();
    void close();
    bool exit();
};