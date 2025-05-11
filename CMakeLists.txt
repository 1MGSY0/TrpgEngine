cmake_minimum_required(VERSION 3.16)
project(TRPGEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# -------------------- GLOBAL INCLUDES --------------------

# GLAD
add_library(glad STATIC packages/glad/src/glad.c)
target_include_directories(glad PUBLIC packages/glad/include)

# ImGui
file(GLOB IMGUI_SRC
    packages/imgui/*.cpp
    packages/imgui/backends/imgui_impl_glfw.cpp
    packages/imgui/backends/imgui_impl_opengl3.cpp
    packages/imgui/misc/cpp/imgui_stdlib.cpp
)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    packages/imgui
    packages/imgui/backends
    packages/imgui/misc/cpp
)

# GLFW (precompiled)
find_library(GLFW3_LIBRARY
    NAMES glfw3
    PATHS ${CMAKE_SOURCE_DIR}/packages/glfw/lib-vc2022
    REQUIRED
)

add_library(glfw3 STATIC IMPORTED GLOBAL)
set_target_properties(glfw3 PROPERTIES
    IMPORTED_LOCATION ${GLFW3_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/packages/glfw/include
)

# Optional: Shared includes
include_directories(
    ${CMAKE_SOURCE_DIR}/packages/glad/include
    ${CMAKE_SOURCE_DIR}/packages/glfw/include
    ${CMAKE_SOURCE_DIR}/packages/imgui
    ${CMAKE_SOURCE_DIR}/packages/nlohmann
)

# Make available to children
set(THIRD_PARTY_LIBS glad imgui glfw3 opengl32)

# -------------------- SUBDIRECTORIES --------------------
add_subdirectory(TRPGEngine)
add_subdirectory(Runtime)
