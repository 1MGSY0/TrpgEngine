# Runtime/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(TRPGRuntime)

set(CMAKE_CXX_STANDARD 17)

# Correct glob path relative to top-level
file(GLOB RUNTIME_SRC
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Runtime/*.cpp
)

add_executable(TRPGRuntime ${RUNTIME_SRC})

target_include_directories(TRPGRuntime PRIVATE
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src
    ${CMAKE_SOURCE_DIR}/packages/nlohmann
)

# Optional: link any required libraries (e.g., if OpenGL used)
# target_link_libraries(TRPGRuntime ...)

# (Optional) Set runtime output directory
set_target_properties(TRPGRuntime PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
