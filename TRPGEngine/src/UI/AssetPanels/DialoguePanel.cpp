#include "DialoguePanel.h"
#include "Engine/Entity/Components/DialogueComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "UI/ImGuiUtils/ImGuiUtils.h"
#include "UI/EditorUI.h"
#include <misc/cpp/imgui_stdlib.h> 

#include <imgui.h>
#include <fstream>
#include <memory>
#include <cstring>
#include <filesystem>

void renderDialogueInspector(std::shared_ptr<DialogueComponent> dialogue) {
    if (!dialogue) {
        ImGui::Text("No text selected.");
        return;
    }

    // Editable name
    ImGui::InputText("Name", &dialogue->name);

    // Editable text
    static std::vector<char> textBuffer;
    // Join lines into a single string separated by newlines
    std::string joinedLines;
    for (size_t i = 0; i < dialogue->lines.size(); ++i) {
        joinedLines += dialogue->lines[i];
        if (i + 1 < dialogue->lines.size()) joinedLines += '\n';
    }
    if (textBuffer.size() < joinedLines.size() + 1024) // ensure buffer is large enough
        textBuffer.resize(joinedLines.size() + 1024);
    std::strncpy(textBuffer.data(), joinedLines.c_str(), textBuffer.size());
    if (ImGui::InputTextMultiline("Text", textBuffer.data(), textBuffer.size(), ImVec2(0, 100))) {
        // Split the buffer back into lines
        std::vector<std::string> newLines;
        std::string bufStr(textBuffer.data());
        size_t pos = 0, prev = 0;
        while ((pos = bufStr.find('\n', prev)) != std::string::npos) {
            newLines.push_back(bufStr.substr(prev, pos - prev));
            prev = pos + 1;
        }
        newLines.push_back(bufStr.substr(prev));
        dialogue->lines = newLines;
    }

    // Editable isNarration
    ImGui::Checkbox("Is Narration", &dialogue->isNarration);

    // Save button
    if (ImGui::Button("Save")) {
        std::string filePath = dialogue->name + ".json";
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << dialogue->toJson().dump(4);
            file.close();
            ImGui::Text("Saved to %s", filePath.c_str());
        } else {
            ImGui::Text("Failed to save to %s", filePath.c_str());
        }
    }
}