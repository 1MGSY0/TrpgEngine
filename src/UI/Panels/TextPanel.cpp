#include "TextPanel.h"
#include "Assets/TextAsset.h"
#include "Project/ResourceManager.h"
#include <imgui.h>
#include <memory>

static char titleBuffer[128] = "";
static char contentBuffer[1024] = "";

void renderTextPanel() {
    ImGui::Text("Text Assets");
    ImGui::InputText("Title", titleBuffer, IM_ARRAYSIZE(titleBuffer));
    ImGui::InputTextMultiline("Content", contentBuffer, IM_ARRAYSIZE(contentBuffer), ImVec2(-FLT_MIN, 100));

    if (ImGui::Button("Add Text")) {
        auto asset = std::make_shared<TextAsset>(titleBuffer, contentBuffer);
        ResourceManager::get().addText(asset);
        titleBuffer[0] = contentBuffer[0] = '\0';
    }

    ImGui::Separator();
    ImGui::Text("Stored Texts:");
    for (auto& text : ResourceManager::get().getTexts()) {
        ImGui::BulletText("%s", text->getName().c_str());
    }
}
