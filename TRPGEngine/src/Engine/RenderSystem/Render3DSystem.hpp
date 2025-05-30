#pragma once
#include "Engine/EntitySystem/Entity.hpp"

class Render3DSystem {
public:
    static void init();                 // Load shaders, set OpenGL state
    static void beginScene();          // Setup camera, clear buffers
    static void renderEntityEditor(Entity e); // Render in editor mode
    static void renderEntityRuntime(Entity e); // Render in runtime mode
    static void endScene();            // Swap buffers if standalone
    static void shutdown();            // Cleanup VAOs, shaders, etc.
};
