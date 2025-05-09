#include "AudioPanel.h"
#include "Engine/Entity/Components/AudioComponent.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Assets/ImportManager.h"

#include <imgui.h>
#include <imgui_stdlib.h>

static std::string name, audioPath;
static float volume = 1.0f;
static bool loop = false;

void renderAudioPanel() {
    ImGui::Text("Audio Component");
    ImGui::InputText("Name", &name);
    ImGui::InputText("File Path", &audioPath);
    ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f);
    ImGui::Checkbox("Loop", &loop);

    if (ImGui::Button("Add Audio Component")) {
        auto audio = std::make_shared<AudioComponent>(name, name);
        audio->setAudioPath(audioPath);
        audio->setLoop(loop);
        audio->setVolume(volume);

        ResourceManager::get().addAudio(name, audio);

        name.clear();
        audioPath.clear();
        volume = 1.0f;
        loop = false;
    }

    ImGui::Separator();
    ImGui::Text("Stored Audio:");

    for (const auto& a : ResourceManager::get().getAllAudio()) {
        ImGui::BulletText("%s", a->getName().c_str());
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATHS")) {
            const char* path = static_cast<const char*>(payload->Data);
            ImportManager::importAsset(path, AssetType::Audio);
        }
        ImGui::EndDragDropTarget();
    }
}

void renderAudioInspector(const std::string& name) {
    auto audio = ResourceManager::get().getAudio(name);
    if (!audio) return;

    ImGui::Text("Editing Audio: %s", name.c_str());

    ImGui::InputText("Audio Path", &audio->getPathRef());
    ImGui::SliderFloat("Volume", &audio->getVolumeRef(), 0.0f, 1.0f);
    ImGui::Checkbox("Loop", &audio->getLoopRef());
}
