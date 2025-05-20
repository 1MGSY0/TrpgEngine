cmake_minimum_required(VERSION 3.16)
project(TRPGEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# -------------------- GLOBAL INCLUDES --------------------

# GLAD
add_library(glad STATIC packages/glad/src/glad.c)
target_include_directories(glad PUBLIC packages/glad/include)

# GLM (header-only)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_SOURCE_DIR}/packages/glm)

# stb (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/packages/stb)

# Assimp
find_library(ASSIMP_LIBRARY
    NAMES assimp assimp-vc143-mt
    PATHS ${CMAKE_SOURCE_DIR}/packages/assimp/lib
    REQUIRED
)

add_library(assimp STATIC IMPORTED GLOBAL)
set_target_properties(assimp PROPERTIES
    IMPORTED_LOCATION ${ASSIMP_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/packages/assimp/include
)


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

# Lua (precompiled)
add_library(lua54 SHARED IMPORTED GLOBAL)

set_target_properties(lua54 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/packages/lua/lua54.dll"
    IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/packages/lua/lua54.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/packages/lua/include"
)

# sol2 (header-only)
add_library(sol INTERFACE)
target_include_directories(sol INTERFACE ${CMAKE_SOURCE_DIR}/packages/sol2/include)

# Optional: Shared includes
include_directories(
    ${CMAKE_SOURCE_DIR}/packages/glad/include
    ${CMAKE_SOURCE_DIR}/packages/glfw/include
    ${CMAKE_SOURCE_DIR}/packages/imgui
    ${CMAKE_SOURCE_DIR}/packages/nlohmann
)

# Make available to children
set(THIRD_PARTY_LIBS glad imgui glfw3 opengl32 glm stb assimp lua54 sol)

# -------------------- SUBDIRECTORIES --------------------
add_subdirectory(TRPGEngine)
add_subdirectory(Runtime)
