#include "Window.h"

#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "implot.h"
#include <stdexcept>
#include "usb2can.h"
#include <string>
#include "globals.h"

Window::Window(int width, int height, const char* title) : width(width), height(height) {
    // Initialize GLFW
    if (!glfwInit()) 
        throw std::runtime_error("Failed to initialize GLFW");

    pWindow = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!pWindow) {
        glfwTerminate();
        throw std::runtime_error("Could not open window");
    }

    glfwMakeContextCurrent(pWindow);
    glfwSwapInterval(1); // Enable VSync

    // Initialize ImGui
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

Window::~Window() {
    // close();
}

void Window::update() {
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Get the dimensions of the GLFW window
    glfwGetFramebufferSize(pWindow, &width, &height);

    createImGui();

    // Render ImGui
    ImGui::Render();
    glViewport(0, 0, width, height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(pWindow);
}

void Window::close() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}

bool Window::exit() {
    return glfwWindowShouldClose(pWindow);
}

void Window::createImGui() {    
    // Set the next window's position and size to fill the entire window
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)), ImGuiCond_Always);

    // Create an ImGui window with no decorations to make it seamless
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::Begin("CANVis", nullptr, window_flags);

    if (ImGui::BeginTabBar("Tabs")) {
        if (ImGui::BeginTabItem("Settings")) createSettingsTab();
        if (ImGui::BeginTabItem("Database")) createDatabaseTab();
        if (ImGui::BeginTabItem("Monitor")) createMonitorTab();
        if (ImGui::BeginTabItem("Graph View")) createGraphTab();

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Window::createSettingsTab() {
    ImGui::Text("Device ID:");
    ImGui::SameLine(115);
    ImGui::Text("Baudrate:");

    ImGui::SetNextItemWidth(100);
    static char deviceID[128] = "";
    ImGui::InputText("##DeviceID", deviceID, IM_ARRAYSIZE(deviceID));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(65);

    if (ImGui::BeginCombo("##Dropdown", std::to_string(baudrate).c_str())) {
        for (int b : baudrates) {
            bool is_selected = (baudrate == b);
            if (ImGui::Selectable(std::to_string(b).c_str(), is_selected))
                baudrate = b;

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    static std::string connectInfo = "";
    if (ImGui::Button("Connect")) {
        std::string configStr = (std::string)deviceID + ";" + std::to_string(baudrate);

        handle = CanalOpen(configStr.c_str(), 0x00000000);
        if (handle <= 0) connectInfo = "CAN Channel not found! ERROR: " + std::to_string(handle);
        else connectInfo = "Connected!";
    }

    ImGui::SameLine();

    ImGui::Text("%s", connectInfo.c_str());

    ImGui::EndTabItem();
}

void Window::createDatabaseTab() {
    if (ImGui::BeginTable("Table1", 3)) // Start a table with 3 columns
        {
            // Set up columns
            ImGui::TableSetupColumn("Column 1");
            ImGui::TableSetupColumn("Column 2");
            ImGui::TableSetupColumn("Column 3");
            ImGui::TableHeadersRow(); // Optional: Adds a header row with column names

            // Add rows
            for (int row = 0; row < 5; ++row)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; ++column)
                {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("Row %d Col %d", row, column);
                }
            }
            ImGui::EndTable(); // End the table
        }

    ImGui::EndTabItem();
}

void Window::createMonitorTab() {

    ImGui::EndTabItem();
}

void Window::createGraphTab() {
    if (ImPlot::BeginPlot("Data Plot")) {
        if (!tData.empty()) { // Ensure there is data to plot
            ImPlot::SetupAxesLimits(tData.front(), tData.back(), -1.0, 1.0, ImGuiCond_Always);
            ImPlot::PlotLine("Acc X", tData.data(), xData.data(), tData.size());
            ImPlot::PlotLine("Acc Y", tData.data(), yData.data(), tData.size());
            ImPlot::PlotLine("Acc Z", tData.data(), zData.data(), tData.size());
        }
        ImPlot::EndPlot();
    }
    ImGui::EndTabItem();
}