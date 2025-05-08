cmake_minimum_required(VERSION 3.16)
project(TRPGRuntime)

set(CMAKE_CXX_STANDARD 17)

add_executable(TRPGRuntime
    main.cpp
    RuntimeApp.cpp
    DataLoader.cpp
)

target_include_directories(TRPGRuntime PRIVATE
    ${CMAKE_SOURCE_DIR}/packages/nlohmann
    ${CMAKE_CURRENT_SOURCE_DIR}
)
