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
#include <sstream>
#include <iomanip>
#include <chrono>
#undef UNICODE
#include <windows.h>
#include <commdlg.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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
    float tabBarY = ImGui::GetCursorPosY();

    if (ImGui::BeginTabBar("Tabs")) {
        if (ImGui::BeginTabItem("Settings")) createSettingsTab();
        if (ImGui::BeginTabItem("Database")) createDatabaseTab();
        if (ImGui::BeginTabItem("Monitor")) createMonitorTab();
        if (ImGui::BeginTabItem("Graph View")) createGraphTab();

        ImGui::EndTabBar();
    }



    // Get the window content region width
    float windowWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SameLine();

    // Get the button size
    float buttonWidth = ImGui::CalcTextSize(isPaused ? "Record" : "Stop").x;
    float rightAlignPadding = 8.0f;

    // Align the button to the right
    ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionMax().x - buttonWidth - rightAlignPadding, tabBarY));

    if (ImGui::Button(isPaused ? "Record" : "Stop")) {
        isPaused = !isPaused; // Toggle the pause state
    }

    ImGui::End();
}

void Window::createSettingsTab() {
    ImGui::Text("Device ID:");
    ImGui::SameLine(115);
    ImGui::Text("Baudrate:");

    ImGui::SetNextItemWidth(100);
    static char deviceID[128] = "AE904396";
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

template< typename T >
static std::string int_to_hex( T i , const int digits, bool add0x = true)
{
  std::stringstream stream;
  if (add0x) stream << "0x";
  stream << std::setfill ('0') << std::setw(digits) 
         << std::hex << i;
  return stream.str();
}

void Window::createDatabaseTab() {
    ImGui::Text("ID:");
    ImGui::SameLine(117);
    ImGui::Text("Name:");
    ImGui::SameLine(325);
    ImGui::Text("Length:");

    ImGui::SetNextItemWidth(100);
    static int messageID = 0;
    const int step = 1;
    const int stepFast = 100;
    ImGui::InputScalar("##messageID", ImGuiDataType_U32, (void*)&messageID, (void*)&step, (void*)&stepFast, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(200);
    static char messageName[128] = "";
    ImGui::InputText("##messageName", messageName, IM_ARRAYSIZE(messageName));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    static int messageLength = 0;
    ImGui::InputScalar("##messageLength", ImGuiDataType_U32, (void*)&messageLength, (void*)&step, (void*)&stepFast);
    ImGui::SameLine();

    static CAN::MessageDescription* selectedDescription = nullptr;
    if ((ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered()) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        selectedDescription = nullptr;
        messageID = 0;
        strcpy_s(messageName, "");
        messageLength = 0;
    }

    if (ImGui::Button("Save")) {
        CAN::MessageDescription messageDescription;
        if (selectedDescription) {
            messageDescription = *selectedDescription;

            if (messageID != selectedDescription->id) {
                messageDescriptions.erase(selectedDescription->id);
            }
        }

        messageDescription.id = messageID;
        messageDescription.name = messageName;
        messageDescription.length = messageLength;

        messageDescriptions[messageID] = messageDescription;
        selectedDescription = &messageDescriptions[messageID];
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
        if (selectedDescription) {
            messageDescriptions.erase(selectedDescription->id);
            selectedDescription = nullptr;
        }
    }

    std::string buttonText = "Import";
    ImVec2 textSize = ImGui::CalcTextSize(buttonText.c_str());
    float availableWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SameLine(availableWidth - textSize.x);
    static std::string dbcFile = "";
    if (ImGui::Button(buttonText.c_str())) {
        dbcFile = openFileDialog();
        if (!dbcFile.empty()) CAN::parseDBC(dbcFile, messageDescriptions);
    }

    if (ImGui::BeginTable("Database", 5, ImGuiTableFlags_RowBg)) // Start a table with 3 columns
        {
            // Set up columns
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200);
            ImGui::TableSetupColumn("Length", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Sender", ImGuiTableColumnFlags_WidthFixed, 200);
            ImGui::TableSetupColumn("Signals");
            ImGui::TableHeadersRow(); // Optional: Adds a header row with column names

            for (auto& pair : messageDescriptions) {
                CAN::MessageDescription& message = pair.second;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", int_to_hex(message.id, 2).c_str());
                ImGui::SameLine();
                if (ImGui::Selectable(("##" + std::to_string(message.id)).c_str(), selectedDescription == &message, ImGuiSelectableFlags_SpanAllColumns)) {
                    selectedDescription = &message;

                    messageID = message.id;
                    strcpy_s(messageName, message.name.c_str());
                    messageLength = message.length;

                }
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", message.name.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%zu", message.length);
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", message.sender.c_str());
                ImGui::TableSetColumnIndex(4);
                std::string signals = "";
                for (CAN::SignalDescription& signal : message.signals) {
                    signals += signal.name + " (" + signal.unit + ")";
                    if (&signal != &message.signals.back()) signals += ", ";
                }
                ImGui::Text("%s", signals.c_str());
            }

            ImGui::EndTable(); // End the table
        }

    ImGui::EndTabItem();
}

void Window::createMonitorTab() {
    if (ImGui::BeginChild("ScrollableTable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_NoBackground)) {
        if (ImGui::BeginTable("Monitor", 6, ImGuiTableFlags_RowBg)) {
            // Set up columns
            ImGui::TableSetupColumn("Timestamp", ImGuiTableColumnFlags_WidthFixed, 100);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 50);
            ImGui::TableSetupColumn("Raw Data", ImGuiTableColumnFlags_WidthFixed, 200);
            ImGui::TableSetupColumn("Data");
            ImGui::TableHeadersRow(); // Optional: Adds a header row with column names

        // for (CAN::Message& message : messageBuffer.getMessages()) {
        // const std::vector<CAN::Message>& messages = messageBuffer.getMessages();
        // for (auto rit = messages.rbegin(); rit != messages.rend(); rit++) {
        for (const CAN::Message& message : messageBuffer) {
            // const CAN::Message& message = *rit;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%lu", message.timestamp);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", int_to_hex(message.id, 2).c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%lu", message.flags);

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%i", message.sizeData);

                ImGui::TableSetColumnIndex(4);
                std::ostringstream oss;
                for (size_t i = 0; i < message.rawData.size(); ++i) {
                    oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(message.rawData[i]);
                    if (i < message.rawData.size() - 1) {
                        oss << " ";
                    }
                }
                ImGui::Text("%s", oss.str().c_str());
                
                ImGui::TableSetColumnIndex(5);
                auto it = messageDescriptions.find(message.id);
                if (it != messageDescriptions.end()) {
                    std::string signals = "";
                    CAN::MessageDescription& description = it->second;
                    for (CAN::SignalDescription& signal : description.signals) {
                        signals += signal.name + ": " + std::to_string(message.getDecodedValue<double>(signal.name)) + " " + signal.unit;
                        if (&signal != &description.signals.back()) signals += "\t";
                    }
                    
                    ImGui::Text("%s", signals.c_str());
                }
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }

    ImGui::EndTabItem();
}

void Window::createGraphTab() {
    ImGui::Columns(2, "Columns");
    ImGui::SetColumnWidth(0, 200);
    static int dtGraph = 0;
    static size_t dtCount = 0;
    // ImGui::BeginTable("CheckboxTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)
    if (ImGui::BeginTable("CheckboxTable", 2)) {
        ImGui::TableSetupColumn("Checkbox", ImGuiTableColumnFlags_WidthFixed, 20);
        ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_WidthFixed, 200);

        // ImGui::TableHeadersRow(); // Optional: Header row

        for (auto& pair : messageDescriptions) {
            CAN::MessageDescription& message = pair.second;
            ImGui::TableNextRow();
            
            // Checkbox Column
            ImGui::TableSetColumnIndex(0);
            ImGui::Checkbox(("##checkbox" + std::to_string(message.id)).c_str(), &message.plot);

            // Text Column
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", (int_to_hex(message.id, 2) + " " + message.name).c_str());
        }

        ImGui::EndTable();
    }

    ImGui::NextColumn();
    ImGui::BeginChild("ScrollablePlots", ImVec2(0, 0), true, ImGuiWindowFlags_NoBackground);

    for (auto& pair : messageDescriptions) {
        CAN::MessageDescription& messageDescription = pair.second;
        if (messageDescription.plot) {
            if (ImPlot::BeginPlot((int_to_hex(messageDescription.id, 2) + " " + messageDescription.name).c_str())) {
                ImPlot::SetupAxes("Time (ms)", "", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_None);

                for (CAN::SignalDescription& signal : messageDescription.signals) {
                    auto dataGetter = [](int idx, void* data) -> ImPlotPoint {
                        auto* info = static_cast<std::pair<std::string, const std::deque<CAN::Message*>*>*>(data);
                        const CAN::Message* msg = (*info->second)[idx];
                        messageBuffer.begin();
                        return ImPlotPoint(msg->timestamp, msg->getDecodedValue<double>(info->first));
                    };

                    const std::deque<CAN::Message*>& data = messageBuffer.ofID(messageDescription.id);
                    std::pair<std::string, const std::deque<CAN::Message*>*> info = {signal.name, &data};
                    ImPlot::PlotLineG(signal.name.c_str(), dataGetter, &info, static_cast<int>(data.size()));
                }

                ImPlot::EndPlot();
            }
        }
    }
    ImGui::EndChild();
    ImGui::Columns();

    ImGui::EndTabItem();
}

std::string Window::openFileDialog() {
    char filename[100] = "";

    HWND hwnd = glfwGetWin32Window(pWindow);

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    return ""; // Return an empty string if canceled
}