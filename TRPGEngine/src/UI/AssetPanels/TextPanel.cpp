#include "TextPanel.h"
#include "Engine/Assets/ImportManager.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Entity/Components/TextComponent.h"
#include "UI/ImGUIUtils/ImGuiUtils.h"

#include <imgui.h>
#include <imgui_stdlib.h>

static std::string name;
static std::string content;
static std::string language = "English";
static std::string tag = "default";
static int fontSize = 16;

void renderTextPanel() {
    ImGui::Text("Text Component Editor");

    ImGui::InputText("Name", &name);
    ImGui::InputTextMultiline("Content", &content, ImVec2(-1, 100));
    ImGui::InputText("Language", &language);
    ImGui::InputText("Tag", &tag);
    ImGui::InputInt("Font Size", &fontSize);

    if (ImGui::Button("Add Text Component")) {
        auto text = std::make_shared<TextComponent>();
        text->setName(name);
        text->setContent(content);
        text->setLanguage(language);
        text->setTag(tag);
        text->setFontSize(fontSize);

        // Using name as the ID for now (could be UUID later)
        ResourceManager::get().addText(name, text);

        // Clear input fields
        name.clear();
        content.clear();
        language = "English";
        tag = "default";
        fontSize = 16;
    }

    ImGui::Separator();
    ImGui::Text("Stored Texts:");

    for (const auto& text : ResourceManager::get().getAllTexts()) {
        ImGui::BulletText("%s", text->getName().c_str());
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Lang: %s\nTag: %s\nFont: %d\n\n%s",
                text->getLanguage().c_str(),
                text->getTag().c_str(),
                text->getFontSize(),
                text->getContent().c_str());
    }

    // Drag-and-drop file import
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATHS")) {
            const char* path = static_cast<const char*>(payload->Data);
            if (path)
                ImportManager::importAsset(path, AssetType::Text);
        }
        ImGui::EndDragDropTarget();
    }
}

void renderTextInspector(const std::string& name) {
    auto text = ResourceManager::get().getText(name);
    if (!text) return;

    ImGui::Text("Editing Text Asset: %s", name.c_str());

    ImGui::InputTextMultiline("Content", &text->getContentRef(), ImVec2(-1, 100));
    ImGui::InputText("Language", &text->getLanguageRef());
    ImGui::InputText("Tag", &text->getTagRef());
    ImGui::SliderInt("Font Size", &text->getFontSizeRef(), 8, 48);
}
