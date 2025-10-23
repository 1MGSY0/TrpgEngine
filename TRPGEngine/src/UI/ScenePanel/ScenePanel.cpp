// NO SUPPORT FOR CLICKING, OR INTERACTING YET

#include "UI/ScenePanel/ScenePanel.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/GameplaySystem/GameInstance.hpp"  // + overlay status
// + HUD event handling
#include "Engine/GameplaySystem/FlowExecutor.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Project/ProjectManager.hpp"

#include <glad/glad.h>
#include <imgui.h>
#include <iostream>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include <random>
#include <algorithm>

ScenePanel::ScenePanel() {
    // Initial framebuffer setup with default size
    createFramebuffer(1280, 720);
}

ScenePanel::~ScenePanel() {
    destroyFramebuffer();
}

void ScenePanel::destroyFramebuffer() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_colorTexture) glDeleteTextures(1, &m_colorTexture);
    if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);

    m_fbo = 0;
    m_colorTexture = 0;
    m_depthRbo = 0;
}

void ScenePanel::createFramebuffer(int width, int height) {
    destroyFramebuffer(); // Clean up existing buffers

    // Framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Depth + stencil buffer
    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[ScenePanel] Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ScenePanel::renderScenePanel() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("Scene Panel", nullptr, flags);

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    if (contentSize.x <= 0.0f || contentSize.y <= 0.0f) {
        ImGui::End();
        return;
    }
    if (contentSize.x > 0 && contentSize.y > 0 &&
        (contentSize.x != m_panelSize.x || contentSize.y != m_panelSize.y)) {
        m_panelSize = contentSize;
        createFramebuffer(static_cast<int>(m_panelSize.x), static_cast<int>(m_panelSize.y));
    }

    // Render scene to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, static_cast<int>(m_panelSize.x), static_cast<int>(m_panelSize.y));
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Prime executor in editor mode so overlay can render events outside runtime
    if (!GameInstance::get().isRunning() && FlowExecutor::get().currentFlowNode() == INVALID_ENTITY) {
        FlowExecutor::get().tick(); // bind to current SceneManager node; no advancement occurs
    }
    const bool previewRunning = (FlowExecutor::get().currentFlowNode() != INVALID_ENTITY);

    // Sink any ImGui draws from SceneManager/RenderSystem into an off-screen window,
    // so they don't push the Scene Panel layout.
    ImGui::SetNextWindowPos(ImVec2(-10000.0f, -10000.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGuiWindowFlags sinkFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (ImGui::Begin("##SceneRenderSink", nullptr, sinkFlags)) {
        if (GameInstance::get().isRunning() || previewRunning) {
            SceneManager::get().renderRuntimeScene();
        } else {
            SceneManager::get().renderEditorScene();
        }
    }
    ImGui::End(); // ##SceneRenderSink

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Display framebuffer texture in ImGui
    if (m_colorTexture) {
        ImTextureID texID = (ImTextureID)(intptr_t)m_colorTexture;
        ImGui::Image(texID, m_panelSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    // --- Play/Preview overlay (interactive) ---
    if (GameInstance::get().isRunning() || previewRunning) {
        // NOTE: Do not auto-tick every frame here; it can skip Choice/Dice instantly.
        // We tick only after user actions (Continue, choose option, roll dice).

        ImVec2 winPos = ImGui::GetItemRectMin();
        ImVec2 winSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos(winPos);
        ImGui::SetNextWindowSize(winSize);
        ImGui::SetNextWindowBgAlpha(0.20f);
        ImGuiWindowFlags overlayFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;

        if (ImGui::Begin("SceneOverlayHUD", nullptr, overlayFlags)) {
            auto& em = EntityManager::get();
            Entity currentEvent = SceneManager::get().getCurrentEventEntity();
            Entity currentNode  = SceneManager::get().getCurrentFlowNode();
            auto nodeComp = em.getComponent<FlowNodeComponent>(currentNode);
            int evtIndex = FlowExecutor::get().currentEventIndex();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.95f, 1.0f, 1.0f));
            ImGui::TextUnformatted("Play Mode");
            ImGui::PopStyleColor();

            ImGui::Separator();
            ImGui::TextDisabled("Scene: %s (ID %u) | Event Index: %d",
                nodeComp ? nodeComp->name.c_str() : "<Unknown>", (unsigned)currentNode, evtIndex);

            if (currentEvent == INVALID_ENTITY) {
                ImGui::TextDisabled("No current event to display.");
            } else {
                // Dialogue
                if (auto dlg = em.getComponent<DialogueComponent>(currentEvent)) {
                    ImGui::Separator();
                    ImGui::TextUnformatted("Dialogue");
                    ImGui::Separator();
                    if (dlg->lines.empty()) {
                        ImGui::TextDisabled("(no lines)");
                    } else {
                        for (auto& l : dlg->lines) ImGui::TextWrapped("%s", l.c_str());
                    }
                    ImGui::Dummy(ImVec2(0, 6));
                    if (dlg->advanceOnClick) {
                        if (ImGui::Button("Continue")) {
                            dlg->triggered = true;
                            FlowExecutor::get().tick(); // advance in-scene or to end-of-scene
                            FlowExecutor::get().tick(); // bind new scene (if we hit the end of scene)
                        }
                    } else {
                        ImGui::TextDisabled("AdvanceOnClick disabled.");
                    }
                }
                // Choice
                else if (auto ch = em.getComponent<ChoiceComponent>(currentEvent)) {
                    ImGui::Separator();
                    ImGui::TextUnformatted("Choice");
                    ImGui::Separator();
                    if (ch->options.empty()) {
                        ImGui::TextDisabled("(no options)");
                    } else {
                        for (size_t i = 0; i < ch->options.size(); ++i) {
                            const auto& opt = ch->options[i];
                            // Split "Text -> Target"
                            std::string baseText = opt.text;
                            std::string target;
                            const std::string delim = " -> ";
                            if (size_t pos = baseText.rfind(delim); pos != std::string::npos) {
                                target = baseText.substr(pos + delim.size());
                                baseText = baseText.substr(0, pos);
                            }
                            if (ImGui::Button(baseText.c_str())) {
                                if (!target.empty()) {
                                    if (target.rfind("@Event:", 0) == 0) {
                                        // Select target event for inspection, then advance to next step
                                        try {
                                            Entity ev = (Entity)std::stoull(target.substr(7));
                                            if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(ev);
                                        } catch (...) {}
                                        FlowExecutor::get().tick(); // advance within scene
                                    } else {
                                        // Switch to target scene
                                        Entity meta = ProjectManager::getProjectMetaEntity();
                                        if (auto base = em.getComponent(meta, ComponentType::ProjectMetadata)) {
                                            auto pm = std::static_pointer_cast<ProjectMetaComponent>(base);
                                            for (Entity n : pm->sceneNodes) {
                                                auto fn = em.getComponent<FlowNodeComponent>(n);
                                                if (fn && fn->name == target) {
                                                    SceneManager::get().setCurrentFlowNode(n);
                                                    if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(n);
                                                    break;
                                                }
                                            }
                                        }
                                        FlowExecutor::get().reset();
                                        FlowExecutor::get().tick(); // bind new scene immediately
                                    }
                                } else {
                                    // No explicit target: default to next event in current scene
                                    FlowExecutor::get().tick();
                                }
                            }
                        }
                    }
                }
                // Dice Roll
                else if (auto dr = em.getComponent<DiceRollComponent>(currentEvent)) {
                    ImGui::Separator();
                    ImGui::TextUnformatted("Dice Roll");
                    ImGui::Separator();
                    ImGui::Text("Roll d%d; success if >= %d", dr->sides, dr->threshold);
                    if (ImGui::Button("Roll")) {
                        int sides = std::max(1, dr->sides);
                        std::random_device rd; std::mt19937 gen(rd());
                        std::uniform_int_distribution<int> dist(1, sides);
                        int roll = dist(gen);
                        bool success = (roll >= dr->threshold);
                        ImGui::OpenPopup("Dice Result");
                        ImGui::SetNextWindowSize(ImVec2(260, 0), ImGuiCond_Appearing);
                        // Route based on result
                        const std::string& tgt = success ? dr->onSuccess : dr->onFailure;
                        if (!tgt.empty()) {
                            if (tgt.rfind("@Event:", 0) == 0) {
                                try {
                                    Entity ev = (Entity)std::stoull(tgt.substr(7));
                                    if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(ev);
                                } catch (...) {}
                                // Advance within scene when chaining via @Event
                                FlowExecutor::get().tick();
                            } else {
                                // Switch to scene by name
                                Entity meta = ProjectManager::getProjectMetaEntity();
                                if (auto base = em.getComponent(meta, ComponentType::ProjectMetadata)) {
                                    auto pm = std::static_pointer_cast<ProjectMetaComponent>(base);
                                    for (Entity n : pm->sceneNodes) {
                                        auto fn = em.getComponent<FlowNodeComponent>(n);
                                        if (fn && fn->name == tgt) {
                                            SceneManager::get().setCurrentFlowNode(n);
                                            if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(n);
                                            break;
                                        }
                                    }
                                }
                                FlowExecutor::get().reset();
                                FlowExecutor::get().tick(); // bind new scene
                            }
                        } else {
                            // No targets: default to next event
                            FlowExecutor::get().tick();
                        }
                        // Show simple result notification
                        if (ImGui::BeginPopupModal("Dice Result", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::Text("You rolled: %d (%s)", roll, success ? "Success" : "Failure");
                            if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                        }
                    }
                }
                // Fallback
                else {
                    ImGui::Separator();
                    ImGui::TextDisabled("Unknown event type. It will auto-complete.");
                }
            }
        }
        ImGui::End();
    }

    ImGui::End();
}