#include "UI/FlowPanel/FlowPlayTester.hpp"
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/GameplaySystem/FlowExecutor.hpp" // + bind/tick executor for play preview

// Shared play state and menu-bound controls
static bool g_playing = false;
static Entity g_playCurrent = INVALID_ENTITY;

static Entity PickStartNode() {
    auto& em = EntityManager::get();
    if (EditorUI* ui = EditorUI::get()) {
        Entity sel = ui->getSelectedEntity();
        if (em.hasComponent(sel, ComponentType::FlowNode)) return sel;
    }
    Entity metaEntity = ProjectManager::getProjectMetaEntity();
    auto metaBase = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
    if (metaBase) {
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(metaBase);
        return (meta->startNode != INVALID_ENTITY) ? meta->startNode : INVALID_ENTITY;
    }
    return INVALID_ENTITY;
}

extern "C" {
    bool Editor_Run_IsPlaying() { return g_playing; }
    void Editor_Run_Play() {
        g_playing = true;
        g_playCurrent = PickStartNode();
        if (g_playCurrent != INVALID_ENTITY) {
            FlowExecutor::get().reset();
            SceneManager::get().setCurrentFlowNode(g_playCurrent);
            if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(g_playCurrent);
            // Initial bind so overlay shows immediately (won't advance Choice/Dice due to executor change)
            FlowExecutor::get().tick();
        }
    }
    void Editor_Run_Stop() {
        g_playing = false;
        g_playCurrent = INVALID_ENTITY;
        FlowExecutor::get().reset();
    }
    void Editor_Run_Restart() {
        g_playCurrent = PickStartNode();
        if (g_playCurrent != INVALID_ENTITY) {
            FlowExecutor::get().reset();
            SceneManager::get().setCurrentFlowNode(g_playCurrent);
            if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(g_playCurrent);
            FlowExecutor::get().tick(); // initial bind
        }
    }
}

void FlowPlayTester::Render() {
    auto& em = EntityManager::get();
    Entity metaEntity = ProjectManager::getProjectMetaEntity();
    auto metaBase = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
    if (!metaBase) {
        ImGui::TextDisabled("Project metadata missing.");
        return;
    }

    ImGui::TextDisabled("Play Tester (MVP)");

    if (!g_playing) {
        if (ImGui::Button("Play")) {
            Editor_Run_Play();
        }
    } else {
        if (ImGui::Button("Stop")) {
            Editor_Run_Stop();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            Editor_Run_Restart();
        }
    }

    ImGui::Separator();
    if (!g_playing) {
        ImGui::TextDisabled("Press Play to start from the selected or Start node.");
        return;
    }
    if (g_playCurrent == INVALID_ENTITY) {
        ImGui::TextDisabled("No Start node set. Right-click a scene in Hierarchy and Set as Start.");
        return;
    }

    // Do not auto-tick here; HUD actions tick when needed.
    auto fn = em.getComponent<FlowNodeComponent>(g_playCurrent);
    const char* missing = "[Missing FlowNode]";
    std::string name = fn ? fn->name : missing;

    ImGui::Text("Current Node: %s (ID: %u)", name.c_str(), (unsigned)g_playCurrent);
    bool isEnd = (fn && fn->isEnd);
    if (isEnd) {
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "End Node");
    }

    ImGui::TextDisabled("Use HUD or Inspector quick triggers to advance events during Play.");
}
