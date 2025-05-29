#pragma once

#include "Engine/EntitySystem/Entity.hpp"

class RenderSystem {
public:
    static void renderEntityEditor(Entity entity);
    static void renderEntityRuntime(Entity entity);

    // Optional: helper for camera setup, batching, etc.
    static void beginScene();
    static void endScene();
};
