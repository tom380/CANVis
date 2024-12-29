cmake_minimum_required(VERSION 3.10)
project(CANVis)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/main.cpp
    src/CAN.cpp
    src/Window.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    implot/implot.cpp
    implot/implot_items.cpp
)

# Include directories
include_directories(
    include
    imgui
    imgui/backends
    imgui/misc/cpp
    implot
)

# Find OpenGL and GLFW
find_package(OpenGL REQUIRED)
# find_package(glfw3 REQUIRED)

# Add executable
add_executable(CANVis ${SOURCES})

target_compile_definitions(CANVis PRIVATE GLEW_STATIC)
target_link_libraries(CANVis
    OpenGL32
    ${CMAKE_SOURCE_DIR}/lib/glfw3.lib
    ${CMAKE_SOURCE_DIR}/lib/glew32s.lib
    ${CMAKE_SOURCE_DIR}/lib/usb2can.lib
)