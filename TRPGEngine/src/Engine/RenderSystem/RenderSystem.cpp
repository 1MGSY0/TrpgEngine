#include "RenderSystem.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"

#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/Transform2DComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"

#include <iostream>

void RenderSystem::beginScene() {
    // Clear screen, setup camera, etc.
}

void RenderSystem::endScene() {
    // Present rendered frame
}

void RenderSystem::renderEntityEditor(Entity e) {
    auto& em = EntityManager::get();

    // render hardcode
    // create new mesh component (assiggn value)
    // 

    if (auto t = em.getComponent<Transform2DComponent>(e)) {
        // Show preview box
        ImGui::GetWindowDrawList()->AddRect(
            ImVec2(t->position.x, t->position.y),
            ImVec2(t->position.x + t->size.x, t->position.y + t->size.y),
            IM_COL32(255, 255, 255, 128)
        );
    }

    if (auto bg = em.getComponent<BackgroundComponent>(e)) {
        // Draw background preview (placeholder)
        ImGui::Text("[Editor] Background: %s", bg->assetPath.c_str());
    }
    if (auto btn = em.getComponent<UIButtonComponent>(e)) {
        ImGui::Button(btn->text.c_str());
    }
    if (auto dlg = em.getComponent<DialogueComponent>(e)) {
        for (const auto& line : dlg->lines) {
            ImGui::TextWrapped("%s", line.c_str());
        }
    }
}

void RenderSystem::renderEntityRuntime(Entity e) {
    auto& em = EntityManager::get();

    if (auto dlg = em.getComponent<DialogueComponent>(e)) {
        if (auto t = em.getComponent<Transform2DComponent>(e)) {
            ImGui::SetCursorScreenPos(ImVec2(t->position.x, t->position.y));
        }

        ImGui::BeginChild(("Dialogue##" + std::to_string(e)).c_str(), ImVec2(400, 100), true);
        for (const auto& line : dlg->lines) {
            ImGui::TextWrapped("%s", line.c_str());
        }

        if (ImGui::IsItemClicked()) {
            dlg->triggered = true;
        }

        // Speaker rendering
        if (dlg->speaker != INVALID_ENTITY) {
            auto speaker = em.getComponent<CharacterComponent>(dlg->speaker);
            if (speaker) {
                ImGui::Text("👤 %s", speaker->name.c_str());
            }
        }

        ImGui::EndChild();
    }


    if (auto btn = em.getComponent<UIButtonComponent>(e)) {
        if (auto t = em.getComponent<Transform2DComponent>(e)) {
            ImGui::SetCursorScreenPos(ImVec2(t->position.x, t->position.y));
        }

        if (ImGui::Button(btn->text.c_str())) {
            btn->triggered = true;
        }
    }

    if (auto bg = em.getComponent<BackgroundComponent>(e)) {
        ImGui::Text("Background Image: %s", bg->assetPath.c_str());
    }
}