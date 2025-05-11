file(GLOB RUNTIME_SRC
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Runtime/*.cpp
)

add_executable(TRPGRuntime ${RUNTIME_SRC})

target_include_directories(TRPGRuntime PRIVATE
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src
)

target_link_libraries(TRPGRuntime
    ${THIRD_PARTY_LIBS}
)
