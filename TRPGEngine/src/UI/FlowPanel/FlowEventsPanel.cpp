#include "UI/FlowPanel/FlowEventsPanel.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <json.hpp>
#include "UI/EditorUI.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/RenderSystem/SceneManager.hpp" // sync viewport with selected scene

void FlowEventsPanel::Render() {
    auto& em = EntityManager::get();
    EditorUI* ui = EditorUI::get();

    // Helper: owner lookup (shared by header and context menu)
    auto findOwnerScene = [&](Entity evt)->Entity {
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (!base) return INVALID_ENTITY;
        auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
        for (Entity n : meta->sceneNodes) {
            auto fn = em.getComponent<FlowNodeComponent>(n);
            if (!fn) continue;
            for (Entity e : fn->eventSequence) if (e == evt) return n;
        }
        return INVALID_ENTITY;
    };

    // Resolve scene context:
    // - If a FlowNode is selected, use it.
    // - If an event is selected, use its owner FlowNode.
    // - Otherwise, fallback to ProjectMeta.startNode.
    Entity selectedNode = INVALID_ENTITY;
    Entity sel = (ui ? ui->getSelectedEntity() : INVALID_ENTITY);
    if (em.hasComponent(sel, ComponentType::FlowNode)) {
        selectedNode = sel;
    } else if (em.getComponent<DialogueComponent>(sel) ||
               em.getComponent<ChoiceComponent>(sel) ||
               em.getComponent<DiceRollComponent>(sel)) {
        selectedNode = findOwnerScene(sel);
    }
    if (selectedNode == INVALID_ENTITY) {
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (base) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
            if (em.hasComponent(meta->startNode, ComponentType::FlowNode))
                selectedNode = meta->startNode;
        }
    }

    // ADD: Per-tab manual override via dropdown (persists until cleared)
    static Entity s_sceneOverride = INVALID_ENTITY;
    {
        // Build scenes list and labels
        std::vector<Entity> sceneIds;
        std::vector<std::string> labels;
        std::vector<const char*> items;
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        if (auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata)) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
            sceneIds.reserve(meta->sceneNodes.size());
            labels.reserve(meta->sceneNodes.size());
            for (Entity nodeId : meta->sceneNodes) {
                sceneIds.push_back(nodeId);
                auto fn = em.getComponent<FlowNodeComponent>(nodeId);
                labels.emplace_back(fn ? fn->name : std::string("[Missing] ") + std::to_string((unsigned)nodeId));
            }
            // Important: build items after labels are finalized to avoid dangling c_str pointers
            items.reserve(labels.size());
            for (auto& s : labels) items.push_back(s.c_str());
        }

        // Prefer override if set
        Entity effectiveNode = (s_sceneOverride != INVALID_ENTITY) ? s_sceneOverride : selectedNode;

        // Compute current index in list
        int curIdx = -1;
        for (int i = 0; i < (int)sceneIds.size(); ++i) {
            if (sceneIds[i] == effectiveNode) { curIdx = i; break; }
        }

        // Scene dropdown
        ImGui::TextDisabled("Scene:");
        ImGui::SameLine();
        if (sceneIds.empty()) {
            ImGui::TextDisabled("<No scenes in project>");
        } else {
            int comboIdx = (curIdx < 0) ? 0 : curIdx;
            if (ImGui::Combo("##FlowEventsSceneCombo", &comboIdx, items.data(), (int)items.size())) {
                s_sceneOverride = sceneIds[comboIdx];
                effectiveNode = s_sceneOverride;
                if (ui) ui->setSelectedEntity(effectiveNode); // sync editor selection to chosen scene
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Follow Selection")) {
                s_sceneOverride = INVALID_ENTITY; // clear override, return to auto context
            }
        }

        // Apply effective scene for the rest of the panel
        selectedNode = effectiveNode;
    }

    // Header
    ImGui::TextDisabled("Events Tab");
    if (selectedNode == INVALID_ENTITY) {
        ImGui::Text("Selected Scene: <None>");
        ImGui::TextDisabled("Tip: Select a FlowNode or an Event in Hierarchy, or pick a Scene above. Use Edit -> Add Event to create events.");
        return;
    } else {
        auto fn = em.getComponent<FlowNodeComponent>(selectedNode);
        std::string nodeName = fn ? fn->name : std::string("[Missing]");
        ImGui::Text("Selected Scene: %s (ID %u)", nodeName.c_str(), (unsigned)selectedNode);
        ImGui::TextDisabled("Use Edit -> Add Event to create events for this scene.");
    }

    auto flow = em.getComponent<FlowNodeComponent>(selectedNode);
    if (!flow) {
        ImGui::TextDisabled("Selected entity has no FlowNodeComponent.");
        return;
    }

    ImGui::Separator();

    // Clean invalid entries helper
    if (ImGui::SmallButton("Clear Invalid Entries")) {
        flow->eventSequence.erase(
            std::remove_if(flow->eventSequence.begin(), flow->eventSequence.end(),
                [&](Entity e){ return e == INVALID_ENTITY; }),
            flow->eventSequence.end()
        );
        ResourceManager::get().setUnsavedChanges(true);
    }

    ImGui::Separator();

    // Move-only helper (reuses findOwnerScene above)
    auto moveEventTo = [&](Entity evt, Entity toNode)->bool {
        if (evt == INVALID_ENTITY || toNode == INVALID_ENTITY) return false;
        Entity owner = findOwnerScene(evt);
        if (owner == toNode) return true;
        if (owner != INVALID_ENTITY) {
            if (auto ownFn = em.getComponent<FlowNodeComponent>(owner)) {
                ownFn->eventSequence.erase(std::remove(ownFn->eventSequence.begin(), ownFn->eventSequence.end(), evt), ownFn->eventSequence.end());
            }
        }
        if (auto dst = em.getComponent<FlowNodeComponent>(toNode)) {
            dst->eventSequence.push_back(evt);
            ResourceManager::get().setUnsavedChanges(true);
            return true;
        }
        return false;
    };

    // Scene list for "Result" editors (Dialogue/Dice)
    std::vector<std::string> sceneNames;
    std::vector<const char*> sceneItems;
    {
        Entity metaEntity = ProjectManager::getProjectMetaEntity();
        auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
        if (base) {
            auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
            sceneNames.emplace_back("<None>");
            for (Entity e : meta->sceneNodes) {
                auto fn = em.getComponent<FlowNodeComponent>(e);
                sceneNames.emplace_back(fn ? fn->name : std::string("[Missing] ") + std::to_string(e));
            }
            for (auto& s : sceneNames) sceneItems.push_back(s.c_str());
        }
    }

    // Events list: left-click selects, right-click context menu, drag-reorder
    static int dragSrcIndex = -1;
    for (int i = 0; i < (int)flow->eventSequence.size(); ++i) {
        Entity evt = flow->eventSequence[i];
        if (evt == INVALID_ENTITY) continue;

        const char* typeLabel = "Unknown";
        std::string preview;
        if (auto d = em.getComponent<DialogueComponent>(evt)) {
            typeLabel = "Dialogue"; if (!d->lines.empty()) preview = d->lines.front();
        } else if (auto c = em.getComponent<ChoiceComponent>(evt)) {
            typeLabel = "Choice"; if (!c->options.empty()) preview = c->options.front().text;
        } else if (auto r = em.getComponent<DiceRollComponent>(evt)) {
            typeLabel = "Dice Roll"; preview = "d" + std::to_string(r->sides) + " >= " + std::to_string(r->threshold);
        }

        ImGui::PushID(i);
        std::string rowText = (i < 10 ? "0" : "") + std::to_string(i) + ". [" + typeLabel + "] ID " + std::to_string((unsigned)evt);
        if (!preview.empty()) rowText += " - " + preview;

        bool isSelected = (ui && ui->getSelectedEntity() == evt);

        // Left-click select (always set selection to refresh Inspector)
        if (ImGui::Selectable(rowText.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
            if (ui) ui->setSelectedEntity(evt);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            if (ui) ui->setSelectedEntity(evt);
        }

        // Ownership hint
        Entity owner = findOwnerScene(evt);
        if (owner != selectedNode && owner != INVALID_ENTITY) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1,0.8f,0.2f,1), "[Owned by Scene %u]", (unsigned)owner);
        }

        // Drag source
        if (ImGui::BeginDragDropSource()) {
            dragSrcIndex = i;
            ImGui::Text("Reorder %d", i);
            ImGui::SetDragDropPayload("EVENT_REORDER_IN_SCENE", &i, sizeof(int));
            ImGui::EndDragDropSource();
        }
        // Drag target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EVENT_REORDER_IN_SCENE")) {
                int src = *(const int*)payload->Data;
                if (src != i && src >= 0 && src < (int)flow->eventSequence.size()) {
                    std::swap(flow->eventSequence[src], flow->eventSequence[i]);
                    ResourceManager::get().setUnsavedChanges(true);
                }
            }
            ImGui::EndDragDropTarget();
        }

        // Right-click context menu: Up / Down / Remove / Move Here (if owned elsewhere)
        if (ImGui::BeginPopupContextItem("evt_ctx")) {
            bool canUp = (i > 0);
            bool canDown = (i < (int)flow->eventSequence.size() - 1);

            if (ImGui::MenuItem("Up", nullptr, false, canUp)) {
                std::swap(flow->eventSequence[i-1], flow->eventSequence[i]);
                ResourceManager::get().setUnsavedChanges(true);
            }
            if (ImGui::MenuItem("Down", nullptr, false, canDown)) {
                std::swap(flow->eventSequence[i+1], flow->eventSequence[i]);
                ResourceManager::get().setUnsavedChanges(true);
            }
            if (ImGui::MenuItem("Remove")) {
                flow->eventSequence.erase(flow->eventSequence.begin() + i);
                ResourceManager::get().setUnsavedChanges(true);
                ImGui::EndPopup();
                ImGui::PopID();
                --i;
                continue;
            }
            if (owner != selectedNode && owner != INVALID_ENTITY) {
                if (ImGui::MenuItem("Move Here")) {
                    moveEventTo(evt, selectedNode);
                }
            }
            ImGui::EndPopup();
        }

        // Inline "Result" editors (same as before)
        if (auto d = em.getComponent<DialogueComponent>(evt)) {
            ImGui::TextDisabled("Result:"); ImGui::SameLine();
            std::string summary = d->targetFlowNode.empty() ? "Next Event or Next Scene (default)" : d->targetFlowNode;
            ImGui::Text("%s", summary.c_str());

            if (!sceneItems.empty()) {
                int cur = 0;
                if (!d->targetFlowNode.empty() && d->targetFlowNode.rfind("@Event:", 0) != 0) {
                    for (int si = 1; si < (int)sceneNames.size(); ++si)
                        if (sceneNames[si] == d->targetFlowNode) { cur = si; break; }
                }
                if (ImGui::Combo("Set Scene##dlg", &cur, sceneItems.data(), (int)sceneItems.size())) {
                    d->targetFlowNode = (cur == 0) ? std::string() : sceneNames[cur];
                    ResourceManager::get().setUnsavedChanges(true);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Clear##dlg")) {
                    d->targetFlowNode.clear();
                    ResourceManager::get().setUnsavedChanges(true);
                }
            }

            // Event target within this scene
            std::vector<std::string> labels; labels.emplace_back("<None>");
            for (Entity e : flow->eventSequence) {
                if (e == INVALID_ENTITY) continue;
                labels.emplace_back("@Event:" + std::to_string((unsigned)e));
            }
            std::vector<const char*> items; for (auto& s : labels) items.push_back(s.c_str());
            int curEvt = 0;
            if (!d->targetFlowNode.empty() && d->targetFlowNode.rfind("@Event:", 0) == 0) {
                for (int k = 1; k < (int)labels.size(); ++k) if (labels[k] == d->targetFlowNode) { curEvt = k; break; }
            }
            if (ImGui::Combo("Set Event##dlg", &curEvt, items.data(), (int)items.size())) {
                d->targetFlowNode = (curEvt == 0) ? std::string() : labels[curEvt];
                ResourceManager::get().setUnsavedChanges(true);
            }
        } else if (auto r = em.getComponent<DiceRollComponent>(evt)) {
            ImGui::TextDisabled("Result:"); ImGui::SameLine();
            std::string succ = r->onSuccess.empty() ? "Next Event/Scene" : r->onSuccess;
            std::string fail = r->onFailure.empty() ? "Next Event/Scene" : r->onFailure;
            ImGui::Text("On Success -> %s | On Failure -> %s", succ.c_str(), fail.c_str());

            if (!sceneItems.empty()) {
                int cs = 0, cf = 0;
                if (!r->onSuccess.empty() && r->onSuccess.rfind("@Event:", 0) != 0)
                    for (int si = 1; si < (int)sceneNames.size(); ++si) if (sceneNames[si] == r->onSuccess) { cs = si; break; }
                if (!r->onFailure.empty() && r->onFailure.rfind("@Event:", 0) != 0)
                    for (int si = 1; si < (int)sceneNames.size(); ++si) if (sceneNames[si] == r->onFailure) { cf = si; break; }

                if (ImGui::Combo("On Success -> Scene##dice", &cs, sceneItems.data(), (int)sceneItems.size())) {
                    r->onSuccess = (cs == 0) ? std::string() : sceneNames[cs];
                    ResourceManager::get().setUnsavedChanges(true);
                }
                if (ImGui::Combo("On Failure -> Scene##dice", &cf, sceneItems.data(), (int)sceneItems.size())) {
                    r->onFailure = (cf == 0) ? std::string() : sceneNames[cf];
                    ResourceManager::get().setUnsavedChanges(true);
                }
            }

            // In-scene event targets
            std::vector<std::string> events; events.emplace_back("<None>");
            for (Entity e : flow->eventSequence) {
                if (e == INVALID_ENTITY) continue;
                events.emplace_back("@Event:" + std::to_string((unsigned)e));
            }
            std::vector<const char*> items; for (auto& s : events) items.push_back(s.c_str());
            int succEvt = 0, failEvt = 0;
            if (!r->onSuccess.empty() && r->onSuccess.rfind("@Event:", 0) == 0)
                for (int i2 = 1; i2 < (int)events.size(); ++i2) if (events[i2] == r->onSuccess) succEvt = i2;
            if (!r->onFailure.empty() && r->onFailure.rfind("@Event:", 0) == 0)
                for (int i2 = 1; i2 < (int)events.size(); ++i2) if (events[i2] == r->onFailure) failEvt = i2;

            if (ImGui::Combo("On Success -> Event", &succEvt, items.data(), (int)items.size())) {
                r->onSuccess = (succEvt == 0) ? std::string() : events[succEvt];
                ResourceManager::get().setUnsavedChanges(true);
            }
            if (ImGui::Combo("On Failure -> Event", &failEvt, items.data(), (int)items.size())) {
                r->onFailure = (failEvt == 0) ? std::string() : events[failEvt];
                ResourceManager::get().setUnsavedChanges(true);
            }
        } else if (em.getComponent<ChoiceComponent>(evt)) {
            ImGui::TextDisabled("Result:"); ImGui::SameLine();
            ImGui::Text("Per choice option. Edit options in Choice inspector.");
        }

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::Separator();

    // Validate branching targets (unchanged)
    if (ImGui::Button("Validate Branching")) {
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
        auto belongsToThisNode = [&](Entity evt) -> bool {
            if (evt == INVALID_ENTITY) return false;
            for (Entity e : flow->eventSequence) if (e == evt) return true;
            return false;
        };
        auto parseEventTag = [&](const std::string& s) -> Entity {
            const std::string tag = "@Event:";
            if (s.rfind(tag, 0) == 0) {
                try { return (Entity)std::stoull(s.substr(tag.size())); } catch (...) { return INVALID_ENTITY; }
            }
            return INVALID_ENTITY;
        };

        int warnings = 0;
        for (Entity evt : flow->eventSequence) {
            if (evt == INVALID_ENTITY) continue;

            if (auto d = em.getComponent<DialogueComponent>(evt)) {
                Entity evtT = parseEventTag(d->targetFlowNode);
                if (evtT != INVALID_ENTITY && !belongsToThisNode(evtT)) ++warnings;
                if (evtT == INVALID_ENTITY && !d->targetFlowNode.empty() && findNodeByName(d->targetFlowNode) == INVALID_ENTITY) ++warnings;
            } else if (auto dice = em.getComponent<DiceRollComponent>(evt)) {
                Entity succT = parseEventTag(dice->onSuccess);
                Entity failT = parseEventTag(dice->onFailure);
                if (succT != INVALID_ENTITY && !belongsToThisNode(succT)) ++warnings;
                if (failT != INVALID_ENTITY && !belongsToThisNode(failT)) ++warnings;
                if (succT == INVALID_ENTITY && !dice->onSuccess.empty() && findNodeByName(dice->onSuccess) == INVALID_ENTITY) ++warnings;
                if (failT == INVALID_ENTITY && !dice->onFailure.empty() && findNodeByName(dice->onFailure) == INVALID_ENTITY) ++warnings;
            } else if (auto ch = em.getComponent<ChoiceComponent>(evt)) {
                for (auto& opt : ch->options) {
                    std::string label = opt.text;
                    const std::string delim = " -> ";
                    size_t pos = label.rfind(delim);
                    if (pos != std::string::npos) {
                        std::string tgt = label.substr(pos + delim.size());
                        Entity evtT = parseEventTag(tgt);
                        if (evtT != INVALID_ENTITY) {
                            if (!belongsToThisNode(evtT)) ++warnings;
                        } else if (!tgt.empty() && findNodeByName(tgt) == INVALID_ENTITY) {
                            ++warnings;
                        }
                    }
                }
            }
        }
        if (warnings == 0) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.4f, 1.0f), "Branching OK: no invalid targets.");
        } else {
            ImGui::TextColored(ImVec4(0.95f, 0.65f, 0.2f, 1.0f), "Branching warnings: %d invalid target(s) found.", warnings);
        }
    }
}
