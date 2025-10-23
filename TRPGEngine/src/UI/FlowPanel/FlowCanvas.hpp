#pragma once
#include <imgui.h>

namespace FlowCanvas {
    // Switch layout orientation. Default is vertical (top-down).
    void UseVerticalLayout(bool enable);

    // Draw the flow canvas.
    void Render();
}
