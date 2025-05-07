cmake_minimum_required(VERSION 3.16)
project(TRPGEngine)

set(CMAKE_CXX_STANDARD 17)

# Enforce static runtime (/MT or /MTd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


# Add include and lib paths manually
# Include GLFW and ImGui
include_directories(
    thirdparty/glfw/include
    thirdparty/imgui
    thirdparty/imgui/backends
)

link_directories(thirdparty/glfw/lib-vc2022)


add_executable(TRPGEngine
    src/main.cpp
    src/Core/Application.cpp

    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(TRPGEngine
    glfw3
    opengl32
)
