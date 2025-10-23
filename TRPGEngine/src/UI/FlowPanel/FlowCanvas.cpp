#define IMGUI_DEFINE_MATH_OPERATORS
#include "UI/FlowPanel/FlowCanvas.hpp"
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"

namespace {
	// Default layout is vertical (top-down)
	bool g_verticalLayout = true;

	// Resolve a scene-name to FlowNode entity
	Entity findSceneByName(const std::string& name) {
		if (name.empty()) return INVALID_ENTITY;
		auto& em = EntityManager::get();
		Entity metaEntity = ProjectManager::getProjectMetaEntity();
		auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
		if (!base) return INVALID_ENTITY;
		auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
		for (Entity n : meta->sceneNodes) {
			auto fn = em.getComponent<FlowNodeComponent>(n);
			if (fn && fn->name == name) return n;
		}
		return INVALID_ENTITY;
	}

	// Collect aggregated targets from events of a scene
	std::map<Entity, std::vector<std::string>> collectAggregatedTargets(Entity sceneNode) {
		std::map<Entity, std::vector<std::string>> agg;
		auto& em = EntityManager::get();
		auto flow = em.getComponent<FlowNodeComponent>(sceneNode);
		if (!flow) return agg;

		const std::string delim = " -> ";

		for (Entity evt : flow->eventSequence) {
			if (evt == INVALID_ENTITY) continue;

			if (auto d = em.getComponent<DialogueComponent>(evt)) {
				// Only scene-name targets; skip @Event jumps
				if (!d->targetFlowNode.empty() && d->targetFlowNode.rfind("@Event:", 0) != 0) {
					Entity tgt = findSceneByName(d->targetFlowNode);
					if (tgt != INVALID_ENTITY) {
						std::string label = "Dialogue";
						if (!d->lines.empty()) label += ": " + d->lines.front();
						agg[tgt].push_back(std::move(label));
					}
				}
			} else if (auto r = em.getComponent<DiceRollComponent>(evt)) {
				if (!r->onSuccess.empty() && r->onSuccess.rfind("@Event:", 0) != 0) {
					Entity tgt = findSceneByName(r->onSuccess);
					if (tgt != INVALID_ENTITY) agg[tgt].push_back("Dice: success");
				}
				if (!r->onFailure.empty() && r->onFailure.rfind("@Event:", 0) != 0) {
					Entity tgt = findSceneByName(r->onFailure);
					if (tgt != INVALID_ENTITY) agg[tgt].push_back("Dice: failure");
				}
			} else if (auto c = em.getComponent<ChoiceComponent>(evt)) {
				for (auto& opt : c->options) {
					std::string baseText = opt.text;
					std::string target;
					size_t pos = baseText.rfind(delim);
					if (pos != std::string::npos) {
						target = baseText.substr(pos + delim.size());
						baseText = baseText.substr(0, pos);
					}
					if (!target.empty() && target.rfind("@Event:", 0) != 0) {
						Entity tgt = findSceneByName(target);
						if (tgt != INVALID_ENTITY) {
							agg[tgt].push_back("Choice: " + baseText);
						}
					}
				}
			}
		}
		return agg;
	}

	// Draw a curved edge with arrowhead; vertical control points when g_verticalLayout
	void drawEdge(ImDrawList* dl, const ImVec2& aCenter, const ImVec2& bCenter, ImU32 color) {
		const float thickness = 2.0f;
		ImVec2 p0 = aCenter;
		ImVec2 p3 = bCenter;
		ImVec2 p1, p2;

		if (g_verticalLayout) {
			p1 = p0 + ImVec2(0.0f, 60.0f);
			p2 = p3 + ImVec2(0.0f, -60.0f);
		} else {
			p1 = p0 + ImVec2(60.0f, 0.0f);
			p2 = p3 + ImVec2(-60.0f, 0.0f);
		}

		dl->AddBezierCubic(p0, p1, p2, p3, color, thickness);

		// Simple arrowhead at end (p3)
		ImVec2 dir = ImVec2(p3.x - p2.x, p3.y - p2.y);
		float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
		if (len > 1e-3f) dir = ImVec2(dir.x / len, dir.y / len);
		const float ah = 8.0f; // arrow size
		ImVec2 left = ImVec2(-dir.y, dir.x);
		ImVec2 a1 = p3 - dir * ah + left * (ah * 0.6f);
		ImVec2 a2 = p3 - dir * ah - left * (ah * 0.6f);
		dl->AddTriangleFilled(p3, a1, a2, color);
	}
} // anonymous namespace

namespace FlowCanvas {
	void UseVerticalLayout(bool enable) { g_verticalLayout = enable; }

	void Render() {
		auto& em = EntityManager::get();
		Entity metaEntity = ProjectManager::getProjectMetaEntity();
		auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
		if (!base) {
			ImGui::TextDisabled("Project metadata missing.");
			return;
		}
		auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

		ImGui::TextDisabled("Flowchart: drag nodes; drag from port to connect; right-click output port to clear link");
		ImGui::BeginChild("FlowchartCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();

		static std::unordered_map<Entity, ImVec2> s_nodePos;
		static Entity s_draggingNode = INVALID_ENTITY;
		static ImVec2 s_dragOffset = ImVec2(0, 0);
		static Entity s_linkFrom = INVALID_ENTITY;
		static bool s_layoutLoaded = false;
		static bool s_layoutDirty = false;

		auto layoutPath = []() -> std::filesystem::path {
			std::filesystem::path proj = ProjectManager::getCurrentProjectPath();
			if (proj.empty()) return {};
			return proj.parent_path() / (proj.stem().string() + ".flowpos.json");
		};

		auto loadLayout = [&]() {
			auto path = layoutPath();
			if (path.empty() || !std::filesystem::exists(path)) return;
			std::ifstream in(path);
			if (!in.is_open()) return;
			nlohmann::json j;
			in >> j;
			if (!j.is_object()) return;
			for (auto it = j.begin(); it != j.end(); ++it) {
				Entity id = static_cast<Entity>(std::stoul(it.key()));
				if (!it.value().is_array() || it.value().size() != 2) continue;
				float x = it.value()[0].get<float>();
				float y = it.value()[1].get<float>();
				s_nodePos[id] = ImVec2(x, y);
			}
		};
		auto saveLayout = [&]() {
			auto path = layoutPath();
			if (path.empty()) return;
			std::error_code ec;
			std::filesystem::create_directories(path.parent_path(), ec);
			nlohmann::json j = nlohmann::json::object();
			for (auto& kv : s_nodePos) {
				j[std::to_string(static_cast<unsigned>(kv.first))] = { kv.second.x, kv.second.y };
			}
			std::ofstream out(path);
			if (!out.is_open()) return;
			out << j.dump(2);
		};

		if (!s_layoutLoaded) {
			loadLayout();
			s_layoutLoaded = true;
		}

		auto ensurePos = [&](Entity e, int idx) {
			if (s_nodePos.find(e) == s_nodePos.end()) {
				float x = 40.0f + (idx % 5) * 200.0f;
				float y = 40.0f + (idx / 5) * 140.0f;
				s_nodePos[e] = ImVec2(x, y);
				s_layoutDirty = true;
			}
		};

		// Links
		for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
			Entity nodeId = meta->sceneNodes[i];
			ensurePos(nodeId, (int)i);
			auto fn = em.getComponent<FlowNodeComponent>(nodeId);
			if (!fn) continue;
			if (fn->nextNode >= 0) {
				Entity to = (Entity)fn->nextNode;
				if (s_nodePos.find(to) == s_nodePos.end()) continue;
				ImVec2 fromPos = origin + s_nodePos[nodeId] + ImVec2(160, 30);
				ImVec2 toPos = origin + s_nodePos[to] + ImVec2(0, 30);
				ImU32 col = IM_COL32(140, 200, 255, 255);
				dl->AddBezierCubic(fromPos, fromPos + ImVec2(50, 0), toPos - ImVec2(50, 0), toPos, col, 2.0f);
			}
		}

		// Nodes
		for (size_t i = 0; i < meta->sceneNodes.size(); ++i) {
			Entity nodeId = meta->sceneNodes[i];
			ensurePos(nodeId, (int)i);
			auto fn = em.getComponent<FlowNodeComponent>(nodeId);
			std::string title = fn ? fn->name : ("[Missing] " + std::to_string(nodeId));

			ImVec2 npos = origin + s_nodePos[nodeId];
			ImVec2 size(160, 60);
			ImVec2 min = npos;
			ImVec2 max = npos + size;

			bool isStart = (meta->startNode == nodeId);
			ImU32 bgCol = isStart ? IM_COL32(60, 140, 80, 255) : IM_COL32(50, 50, 60, 255);
			ImU32 borderCol = IM_COL32(90, 90, 110, 255);
			dl->AddRectFilled(min, max, bgCol, 6.0f);
			dl->AddRect(min, max, borderCol, 6.0f);

			// Title
			dl->AddText(min + ImVec2(8, 8), IM_COL32_WHITE, title.c_str());

			// Event visualization (dots + count)
			if (fn) {
				// collect valid events
				std::vector<Entity> evts;
				evts.reserve(fn->eventSequence.size());
				for (Entity e : fn->eventSequence) if (e != INVALID_ENTITY) evts.push_back(e);

				// label with count
				char countBuf[16]; snprintf(countBuf, sizeof(countBuf), "E:%d", (int)evts.size());
				dl->AddText(min + ImVec2(size.x - 36.0f, 8.0f), IM_COL32(200,200,200,255), countBuf);

				// highlight active if selected entity belongs to this node's events
				int activeIdx = -1;
				if (EditorUI* ui = EditorUI::get()) {
					Entity sel = ui->getSelectedEntity();
					if (sel != INVALID_ENTITY) {
						for (int k = 0; k < (int)evts.size(); ++k) {
							if (evts[k] == sel) { activeIdx = k; break; }
						}
					}
				}

				// draw compact row of dots at bottom
				const float r = 3.5f;
				const float spacing = 8.0f;
				int showN = (int)evts.size();
				// cap to avoid clutter, still show last ones if many
				const int maxDots = 12;
				int startIdx = 0;
				if (showN > maxDots) { startIdx = showN - maxDots; showN = maxDots; }

				float totalW = (showN - 1) * spacing;
				ImVec2 rowCenter = ImVec2(min.x + size.x * 0.5f, max.y - 10.0f);
				ImVec2 rowStart = rowCenter - ImVec2(totalW * 0.5f, 0.0f);

				for (int d = 0; d < showN; ++d) {
					ImVec2 p = ImVec2(rowStart.x + d * spacing, rowStart.y);
					int globalIdx = startIdx + d;
					bool isActive = (activeIdx == globalIdx);
					ImU32 col = isActive ? IM_COL32(255, 215, 100, 255) : IM_COL32(180, 180, 195, 255);
					dl->AddCircleFilled(p, r, col);
				}
			}

			// Ports
			ImVec2 inPort = min + ImVec2(-6, 30);
			ImVec2 outPort = max + ImVec2(6, -30);
			dl->AddCircleFilled(inPort, 6.0f, IM_COL32(200, 200, 200, 255));
			dl->AddCircleFilled(outPort, 6.0f, IM_COL32(200, 200, 200, 255));

			// Drag node
			bool hovered = ImGui::IsMouseHoveringRect(min, max);
			if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				s_draggingNode = nodeId;
				s_dragOffset = ImGui::GetMousePos() - npos;
				if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(nodeId);
				SceneManager::get().setCurrentFlowNode(nodeId);
			}
			if (s_draggingNode == nodeId && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
				ImVec2 newPos = ImGui::GetMousePos() - origin - s_dragOffset;
				if (newPos.x != s_nodePos[nodeId].x || newPos.y != s_nodePos[nodeId].y) {
					s_nodePos[nodeId] = newPos;
					s_layoutDirty = true;
				}
			}
			if (s_draggingNode == nodeId && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				s_draggingNode = INVALID_ENTITY;
			}

			// Start linking from output port
			if (ImGui::IsMouseHoveringRect(outPort - ImVec2(8, 8), outPort + ImVec2(8, 8))) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) s_linkFrom = nodeId;
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					if (auto src = em.getComponent<FlowNodeComponent>(nodeId)) {
						src->nextNode = -1;
						ResourceManager::get().setUnsavedChanges(true);
					}
				}
			}

			// Live link line
			if (s_linkFrom != INVALID_ENTITY) {
				ImVec2 fromPos = origin + s_nodePos[s_linkFrom] + ImVec2(160, 30);
				ImVec2 mousePos = ImGui::GetMousePos();
				dl->AddBezierCubic(fromPos, fromPos + ImVec2(50, 0), mousePos - ImVec2(50, 0), mousePos, IM_COL32(255, 255, 100, 255), 2.0f);
			}

			// Accept link on input port
			if (ImGui::IsMouseHoveringRect(inPort - ImVec2(8, 8), inPort + ImVec2(8, 8))) {
				if (s_linkFrom != INVALID_ENTITY && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
					if (auto src = em.getComponent<FlowNodeComponent>(s_linkFrom)) {
						src->nextNode = (int)nodeId;
						ResourceManager::get().setUnsavedChanges(true);
					}
					s_linkFrom = INVALID_ENTITY;
				}
			}
		}

		if (s_layoutDirty) {
			saveLayout();
			s_layoutDirty = false;
		}

		// Cancel linking if mouse released elsewhere
		if (s_linkFrom != INVALID_ENTITY && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			s_linkFrom = INVALID_ENTITY;
		}

		// Draw aggregated multi-edges (event routes) and explicit nextNode edges
		for (Entity src : meta->sceneNodes) {
			if (!em.getComponent<FlowNodeComponent>(src)) continue;

			// Skip if center missing
			// if (!nodeCenters.count(src)) continue;

			// Aggregate event-driven targets for this scene
			auto agg = collectAggregatedTargets(src);
			if (!agg.empty()) {
				// ImVec2 srcCenter = nodeCenters[src];
				// Replace with your actual center retrieval:
				ImVec2 srcCenter = /* nodeCenters[src] */ ImVec2(0, 0); // ...existing code...

				for (auto& kv : agg) {
					Entity dst = kv.first;
					const auto& labels = kv.second;
					if (dst == INVALID_ENTITY) continue;
					// if (!nodeCenters.count(dst)) continue;
					ImVec2 dstCenter = /* nodeCenters[dst] */ ImVec2(0, 0); // ...existing code...

					// Soft blue for aggregated routes
					drawEdge(dl, srcCenter, dstCenter, IM_COL32(180, 210, 255, 220));

					// Hover tooltip: list contributing events (simple midpoint hit box)
					ImVec2 mid = (srcCenter + dstCenter) * 0.5f;
					ImRect hit(mid - ImVec2(8, 8), mid + ImVec2(8, 8));
					if (hit.Contains(ImGui::GetIO().MousePos)) {
						ImGui::BeginTooltip();
						ImGui::Text("Routes:");
						for (auto& s : labels) ImGui::BulletText("%s", s.c_str());
						ImGui::EndTooltip();
					}
				}
			}

			// Explicit nextNode edge (distinct color)
			if (auto fn = em.getComponent<FlowNodeComponent>(src)) {
				if (fn->nextNode != INVALID_ENTITY) {
					// if (!nodeCenters.count((Entity)fn->nextNode)) continue;
					// ImVec2 srcCenter = nodeCenters[src];
					// ImVec2 dstCenter = nodeCenters[(Entity)fn->nextNode];
					ImVec2 srcCenter = /* nodeCenters[src] */ ImVec2(0, 0);              // ...existing code...
					ImVec2 dstCenter = /* nodeCenters[(Entity)fn->nextNode] */ ImVec2(0, 0); // ...existing code...
					drawEdge(dl, srcCenter, dstCenter, IM_COL32(180, 255, 180, 220));
				}
			}
		}

		ImGui::EndChild();
	}
}
