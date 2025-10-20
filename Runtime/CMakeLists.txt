# --- Gather source files ---
file(GLOB RUNTIME_SRC
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Runtime/*.cpp
)

# --- Create executable ---
add_executable(TRPGRuntime
    ${RUNTIME_SRC}
)

# --- Include paths ---
target_include_directories(TRPGRuntime PRIVATE
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src
)

# --- Link libraries ---
target_link_libraries(TRPGRuntime
    ${THIRD_PARTY_LIBS}
)

# --- Post-build steps ---
add_custom_command(TARGET TRPGRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/Runtime
            $<TARGET_FILE_DIR:TRPGRuntime>/Runtime

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/packages/lua/lua54.dll
            $<TARGET_FILE_DIR:TRPGRuntime>/lua54.dll
)
