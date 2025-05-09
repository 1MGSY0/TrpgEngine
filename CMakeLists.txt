cmake_minimum_required(VERSION 3.16)
project(TRPGEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# Add subdirectories
add_subdirectory(TRPGEngine)
add_subdirectory(Runtime)
