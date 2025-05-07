#include "AudioPanel.h"
#include "Assets/AudioAsset.h"
#include "Project/ResourceManager.h"
#include <imgui.h>
#include <memory>

static char nameBuffer[128] = "";
static char pathBuffer[256] = "";

void renderAudioPanel() {
    ImGui::Text("Audio Assets");
    ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer));
    ImGui::InputText("File Path", pathBuffer, IM_ARRAYSIZE(pathBuffer));

    if (ImGui::Button("Import Audio")) {
        auto asset = std::make_shared<AudioAsset>(nameBuffer, pathBuffer);
        ResourceManager::get().addAudio(asset);
        nameBuffer[0] = pathBuffer[0] = '\0';
    }

    ImGui::Separator();
    ImGui::Text("Imported Audio Files:");
    for (auto& audio : ResourceManager::get().getAudios()) {
        ImGui::BulletText("%s -> %s", audio->getName().c_str(), audio->getFilePath().c_str());
    }
}
