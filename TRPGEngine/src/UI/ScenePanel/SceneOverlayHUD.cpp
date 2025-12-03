#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "UI/ScenePanel/SceneOverlayHUD.hpp"
#include <random>
#include <unordered_map>
#include <algorithm> 
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Project/ProjectManager.hpp"

void SceneOverlayHUD::Render() {
    // Attach overlay to the Scene Panel window if it exists
    ImGuiWindow* sceneWin = ImGui::FindWindowByName("Scene Panel");
    if (!sceneWin) return;

    // If another overlay with the legacy name exists this frame, skip to avoid double-Begin clash
    if (ImGui::FindWindowByName("SceneOverlayHUD")) {
        return;
    }

    ImGui::SetNextWindowPos(sceneWin->Pos);
    ImGui::SetNextWindowSize(sceneWin->Size);
    ImGui::SetNextWindowBgAlpha(0.20f);
    ImGuiWindowFlags overlayFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    // Use a unique name to avoid any Begin/End conflicts with legacy overlays
    if (!ImGui::Begin("Scene HUD Overlay", nullptr, overlayFlags)) {
        ImGui::End();
        return;
    }

    auto& em = EntityManager::get();
    EditorUI* ui = EditorUI::get();
    if (!ui) { ImGui::End(); return; }

    Entity current = ui->getSelectedEntity();
    bool hasFlow = em.hasComponent(current, ComponentType::FlowNode);
    if (!hasFlow) {
        // Try Project Start as fallback
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (base) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
            if (meta->startNode != INVALID_ENTITY) {
                current = meta->startNode;
                hasFlow = em.hasComponent(current, ComponentType::FlowNode);
            }
        }
    }

    if (!hasFlow) {
        ImGui::TextDisabled("Select a Flow Node to preview.");
        ImGui::End();
        return;
    }

    auto fn = em.getComponent<FlowNodeComponent>(current);
    std::string name = fn ? fn->name : std::string("[Missing]");
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.98f, 1.0f, 1.0f));
    ImGui::Text("Node: %s (ID %u)", name.c_str(), (unsigned)current);
    ImGui::PopStyleColor();

    bool isEnd = (fn && fn->isEnd);
    if (isEnd) {
        ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "End Node");
    }

    // Minimal Continue
    ImGui::BeginDisabled(isEnd || !fn);
    if (ImGui::Button("Continue")) {
        if (fn && fn->nextNode >= 0) {
            Entity next = (Entity)fn->nextNode;
            ui->setSelectedEntity(next);
            SceneManager::get().setCurrentFlowNode(next);
        }
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    // Keep per-node event playback state
    static std::unordered_map<Entity, Entity> s_currentEventByNode; // node -> current event entity
    static Entity s_lastNode = INVALID_ENTITY;

    auto getFirstValidEvent = [&](const std::shared_ptr<FlowNodeComponent>& nodeComp) -> Entity {
        if (!nodeComp) return INVALID_ENTITY;
        for (Entity e : nodeComp->eventSequence) if (e != INVALID_ENTITY) return e;
        return INVALID_ENTITY;
    };

    // Reset current event when node changes
    if (s_lastNode != current) {
        s_lastNode = current;
        Entity first = getFirstValidEvent(fn);
        s_currentEventByNode[current] = first;
        if (first != INVALID_ENTITY) {
            ui->setSelectedEntity(first); // select first event on entering node
        }
    }

    // Helper: find FlowNode by name
    auto findNodeByName = [&](const std::string& nodeName) -> Entity {
        if (nodeName.empty()) return INVALID_ENTITY;
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (!base) return INVALID_ENTITY;
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
        for (Entity e : meta->sceneNodes) {
            auto f = em.getComponent<FlowNodeComponent>(e);
            if (f && f->name == nodeName) return e;
        }
        return INVALID_ENTITY;
    };

    auto parseEventTag = [&](const std::string& s) -> Entity {
        // expects "@Event:<id>"
        const std::string tag = "@Event:";
        if (s.rfind(tag, 0) == 0) {
            try {
                uint64_t id = std::stoull(s.substr(tag.size()));
                return (Entity)id;
            } catch (...) { return INVALID_ENTITY; }
        }
        return INVALID_ENTITY;
    };

    auto advanceToScene = [&](Entity sceneNode) {
        if (sceneNode == INVALID_ENTITY) return;
        ui->setSelectedEntity(sceneNode);
        SceneManager::get().setCurrentFlowNode(sceneNode);
        s_currentEventByNode.erase(sceneNode); // reset next time we enter
    };

    auto advanceToEvent = [&](Entity targetEvent) {
        // Only if this event belongs to current node's eventSequence
        if (!fn || targetEvent == INVALID_ENTITY) return false;
        bool belongs = false;
        for (Entity e : fn->eventSequence) if (e == targetEvent) { belongs = true; break; }
        if (belongs) {
            s_currentEventByNode[current] = targetEvent;
            ui->setSelectedEntity(targetEvent); // select advanced event
            return true;
        }
        return false;
    };

    auto advanceToNextEventInOrder = [&]() {
        if (!fn) return false;
        Entity cur = s_currentEventByNode[current];
        int idx = -1;
        for (int i = 0; i < (int)fn->eventSequence.size(); ++i) {
            if (fn->eventSequence[i] == cur) { idx = i; break; }
        }
        for (int j = idx + 1; j < (int)fn->eventSequence.size(); ++j) {
            if (fn->eventSequence[j] != INVALID_ENTITY) {
                s_currentEventByNode[current] = fn->eventSequence[j];
                ui->setSelectedEntity(fn->eventSequence[j]); // select next event
                return true;
            }
        }
        return false;
    };

    // If no current event, nothing to render; show hint
    Entity curEvent = s_currentEventByNode[current];
    if (curEvent == INVALID_ENTITY) {
        ImGui::TextDisabled("No events in this scene. Use Events tab to add Dialogue/Choice/Dice.");
        ImGui::End();
        return;
    }

    // Render the current event based on its component type
    // Dialogue
    if (auto d = em.getComponent<DialogueComponent>(curEvent)) {
        ImGui::Separator();
        ImGui::TextDisabled("Dialogue");
        static std::unordered_map<Entity, int> s_dialogLine;
        const int total = (int)d->lines.size();
        int& idx = s_dialogLine[curEvent];
        if (idx < 0) idx = 0;
        if (total > 0 && idx >= total) idx = total - 1;

        if (d->speaker != INVALID_ENTITY) {
            if (auto ch = em.getComponent<CharacterComponent>(d->speaker)) {
                ImGui::Text("%s:", ch->name.c_str());
            }
        }
        if (total > 0) ImGui::TextWrapped("%s", d->lines[idx].c_str());
        else ImGui::TextDisabled("(no lines)");

        bool finished = (total == 0) || (idx >= total - 1);
        ImGui::BeginDisabled(total == 0);
        if (ImGui::Button(finished ? "Finish" : "Next")) {
            bool progressed = false;
            if (!finished) {
                idx++; progressed = true;
            } else {
                // First try explicit target event tag
                Entity evtTarget = parseEventTag(d->targetFlowNode);
                if (evtTarget != INVALID_ENTITY && advanceToEvent(evtTarget)) {
                    progressed = true;
                } else {
                    // Scene target by name
                    Entity sceneTarget = findNodeByName(d->targetFlowNode);
                    if (sceneTarget != INVALID_ENTITY) {
                        advanceToScene(sceneTarget);
                        progressed = true;
                    }
                }
                // If no explicit target, go next event; else fallback to scene nextNode
                if (!progressed) {
                    if (!advanceToNextEventInOrder()) {
                        if (fn && fn->nextNode >= 0) {
                            advanceToScene((Entity)fn->nextNode);
                            progressed = true;
                        }
                    }
                }
            }
        }
        ImGui::EndDisabled();
    }
    // DiceRoll
    else if (auto dice = em.getComponent<DiceRollComponent>(curEvent)) {
        ImGui::Separator();
        ImGui::TextDisabled("Dice Check");
        ImGui::Text("Sides: %d  Threshold: %d", dice->sides, dice->threshold);
        static std::unordered_map<Entity, int> s_lastRollFor; // per dice entity
        int& lastRoll = s_lastRollFor[curEvent];
        if (lastRoll > 0) ImGui::Text("Last roll: %d", lastRoll);

        if (ImGui::Button("Roll")) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(1, (std::max)(1, dice->sides));
            lastRoll = dist(gen);
            bool success = (lastRoll >= dice->threshold);
            const std::string& nextName = success ? dice->onSuccess : dice->onFailure;

            // Event target?
            Entity evtTarget = parseEventTag(nextName);
            if (evtTarget != INVALID_ENTITY && advanceToEvent(evtTarget)) {
                // ok
            } else {
                // Scene target?
                Entity sceneTarget = findNodeByName(nextName);
                if (sceneTarget != INVALID_ENTITY) {
                    advanceToScene(sceneTarget);
                } else {
                    // Fallback: next event or scene Next
                    if (!advanceToNextEventInOrder() && fn && fn->nextNode >= 0) {
                        advanceToScene((Entity)fn->nextNode);
                    }
                }
            }
        }
    }
    // Choice
    else if (auto choice = em.getComponent<ChoiceComponent>(curEvent)) {
        ImGui::Separator();
        ImGui::TextDisabled("Choice");
        if (choice->options.empty()) {
            ImGui::TextDisabled("(no options)");
        } else {
            for (size_t i = 0; i < choice->options.size(); ++i) {
                const std::string& raw = choice->options[i].text;
                std::string baseLabel = raw;
                std::string targetName;
                const std::string delim = " -> ";
                size_t pos = baseLabel.rfind(delim);
                if (pos != std::string::npos) {
                    targetName = baseLabel.substr(pos + delim.size());
                    baseLabel = baseLabel.substr(0, pos);
                }

                if (ImGui::Button(baseLabel.c_str())) {
                    Entity evtTarget = parseEventTag(targetName);
                    if (evtTarget != INVALID_ENTITY && advanceToEvent(evtTarget)) {
                        // ok
                    } else {
                        Entity sceneTarget = findNodeByName(targetName);
                        if (sceneTarget != INVALID_ENTITY) {
                            advanceToScene(sceneTarget);
                        } else {
                            if (!advanceToNextEventInOrder() && fn && fn->nextNode >= 0) {
                                advanceToScene((Entity)fn->nextNode);
                            }
                        }
                    }
                }
            }
        }
    } else {
        // Unknown event type, skip to next
        if (!advanceToNextEventInOrder() && fn && fn->nextNode >= 0) {
            advanceToScene((Entity)fn->nextNode);
        }
    }

    ImGui::End();
}
