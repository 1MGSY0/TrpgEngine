#pragma once

#include <imgui.h>

class ScenePanel {
public:
    ScenePanel();
    ~ScenePanel();

    // Main render entry
    void renderScenePanel();

private:
    void createFramebuffer(int width, int height);
    void destroyFramebuffer();

    unsigned int m_fbo = 0;
    unsigned int m_colorTexture = 0;
    unsigned int m_depthRbo = 0;

    ImVec2 m_panelSize = ImVec2(0, 0);
};
