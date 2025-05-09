# --- Gather source files ---
file(GLOB_RECURSE ENGINE_SRC
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Engine/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Engine/*/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/Project/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/UI/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/UI/*/*.cpp
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src/UI/*/*/*.cpp            
)

set(MAIN_SRC ${CMAKE_SOURCE_DIR}/TRPGEngine/src/main.cpp)

# ImGui sources
set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/packages/imgui/misc/cpp/imgui_stdlib.cpp
)

# --- Create executable ---
add_executable(TRPGEngine
    ${MAIN_SRC}
    ${ENGINE_SRC}
    ${IMGUI_SRC}
)

# --- Include paths ---
target_include_directories(TRPGEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/TRPGEngine/src
    ${CMAKE_SOURCE_DIR}/packages/imgui
    ${CMAKE_SOURCE_DIR}/packages/imgui/backends
    ${CMAKE_SOURCE_DIR}/packages/imgui/misc/cpp
    ${CMAKE_SOURCE_DIR}/packages/glfw/include
    ${CMAKE_SOURCE_DIR}/packages/nlohmann
)

# --- Link GLFW and OpenGL ---
find_library(GLFW3_LIBRARY
    NAMES glfw3
    PATHS ${CMAKE_SOURCE_DIR}/packages/glfw/lib-vc2022
    REQUIRED
)

target_link_libraries(TRPGEngine
    ${GLFW3_LIBRARY}
    opengl32
)

# --- Set output directory ---
set_target_properties(TRPGEngine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# --- Post-build: copy assets ---
add_custom_command(TARGET TRPGEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:TRPGEngine>/assets
)
