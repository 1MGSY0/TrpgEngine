cmake_minimum_required(VERSION 3.15)
project(TRPGRuntime)

set(CMAKE_CXX_STANDARD 17)

add_executable(TRPGRuntime
    main.cpp
    RuntimeApp.cpp
    DataLoader.cpp
)

# If json.hpp is not system-wide
target_include_directories(TRPGRuntime PRIVATE ../external/json)