# --- Gather source files ---
file(GLOB_RECURSE ENGINE_SRC
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Engine/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Engine/*/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Project/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Resources/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/UI/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/UI/*/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/UI/*/*/*.cpp            
)

set(MAIN_SRC ${CMAKE_SOURCE_DIR}/TRPGEngine/src/main.cpp)

# --- Create executable ---
add_executable(TRPGEngine
    ${MAIN_SRC}
    ${ENGINE_SRC}
)

# --- Include paths ---
target_include_directories(TRPGEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src
)

# --- Link shared libraries from top-level ---
target_link_libraries(TRPGEngine
    ${THIRD_PARTY_LIBS}
)

# --- Copy assets after build ---
add_custom_command(TARGET TRPGEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:TRPGEngine>/assets
)