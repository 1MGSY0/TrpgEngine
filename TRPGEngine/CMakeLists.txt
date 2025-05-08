cmake_minimum_required(VERSION 3.16)
project(TRPGEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}/packages/glfw/include
    ${CMAKE_SOURCE_DIR}/packages/imgui
    ${CMAKE_SOURCE_DIR}/packages/imgui/backends
    ${CMAKE_SOURCE_DIR}/packages/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link GLFW lib
link_directories(${CMAKE_SOURCE_DIR}/packages/glfw/lib-vc2022)

add_executable(TRPGEngine
    src/main.cpp
    src/Core/Application.cpp
    src/ImGUIUtils/ImGuiUtils.cpp

    src/Assets/Character.cpp
    src/Assets/TextAsset.cpp
    src/Assets/AudioAsset.cpp

    src/Project/ResourceManager.cpp
    src/Project/ProjectManager.cpp
    src/Project/BuildSystem.cpp

    src/UI/ScenePanel/ScenePanel.cpp
    src/UI/AssetPanels/TextPanel.cpp
    src/UI/AssetPanels/CharactersPanel.cpp
    src/UI/AssetPanels/AudioPanel.cpp
    src/UI/Panels/ProjectPanel.cpp
    src/UI/EditorUI.cpp

    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(TRPGEngine
    glfw3
    opengl32
)

# Copy assets
add_custom_command(TARGET TRPGEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:TRPGEngine>/assets
)
