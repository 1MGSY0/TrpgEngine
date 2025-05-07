cmake_minimum_required(VERSION 3.16)
project(TRPGEngine)

set(CMAKE_CXX_STANDARD 17)

# Enforce static runtime (/MT or /MTd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


# Add include and lib paths manually
# Include GLFW and ImGui
include_directories(TRPGEngine PRIVATE
    src
    thirdparty/glfw/include
    thirdparty/imgui
    thirdparty/imgui/backends
    thirdparty/nlohmann
)

link_directories(thirdparty/glfw/lib-vc2022)


add_executable(TRPGEngine
    src/main.cpp
    src/Core/Application.cpp

    src/Assets/Character.cpp
    src/Assets/TextAsset.cpp
    src/Assets/AudioAsset.cpp
    src/Project/ResourceManager.cpp
    src/Project/ProjectManager.cpp

    src/UI/ScenePanel/ScenePanel.cpp

    src/UI/AssetPanels/TextPanel.cpp
    src/UI/AssetPanels/CharactersPanel.cpp
    src/UI/AssetPanels/AudioPanel.cpp
    src/UI/EditorUI.cpp

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

add_custom_command(TARGET TRPGEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:TRPGEngine>/assets)