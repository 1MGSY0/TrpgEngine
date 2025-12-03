#include "RenderSystem.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/Transform2DComponent.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"
#include "Engine/EntitySystem/Components/UIButtonComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/ModelComponent.hpp"
// routing / executor / scene access
#include "Engine/GameplaySystem/FlowExecutor.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/ResourceUtils.hpp"
#include "UI/EditorUI.hpp"
#include "Engine/Graphics/TextureHelpers.hpp"
#include <imgui.h>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <cstring>
#include <cmath>
#include <cctype>
#include <unordered_set>
#include <deque>
#include <sstream>

// Ensure iostream is available for fallback ResourceUtils logging
#include <iostream>

namespace RenderSystem {

void init() {
	// placeholder for future GL/resource init
	std::cout << "[RenderSystem] init\n";
	// Ensure resource placeholders are ready (1x1 texture)
	ResourceUtils::ensureInitialized();
}

void shutdown() {
	std::cout << "[RenderSystem] shutdown\n";
}

struct BackgroundQuad {
	ImVec2 min;
	ImVec2 max;
};

static BackgroundQuad calcBackgroundQuad(Entity entity) {
	ImVec2 winPos  = ImGui::GetWindowPos();
	ImVec2 winSize = ImGui::GetWindowSize();
	BackgroundQuad quad{winPos, ImVec2(winPos.x + winSize.x, winPos.y + winSize.y)};

	auto applyTransform = [&](float scaleX, float scaleY, float offsetX, float offsetY) {
		if (std::fabs(scaleX) < 0.0001f) scaleX = 1.0f;
		if (std::fabs(scaleY) < 0.0001f) scaleY = 1.0f;
		ImVec2 dims(winSize.x * scaleX, winSize.y * scaleY);
		quad.min = ImVec2(winPos.x + offsetX, winPos.y + offsetY);
		quad.max = ImVec2(quad.min.x + dims.x, quad.min.y + dims.y);
	};

	if (entity != INVALID_ENTITY) {
		auto& em = EntityManager::get();
		if (auto t2d = em.getComponent<Transform2DComponent>(entity)) {
			applyTransform(t2d->scale.x, t2d->scale.y, t2d->position.x, t2d->position.y);
			return quad;
		}
		if (auto t3d = em.getComponent<TransformComponent>(entity)) {
			applyTransform(t3d->scale.x, t3d->scale.y, t3d->position.x, t3d->position.y);
		}
	}
	return quad;
}

static void drawTexturedBackground(ImTextureID tex, Entity entity = INVALID_ENTITY) {
	auto quad = calcBackgroundQuad(entity);
	auto dl   = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size= ImGui::GetWindowSize();
	dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
	dl->AddImage(tex, quad.min, quad.max, ImVec2(0,1), ImVec2(1,0));
	dl->PopClipRect();
}

static void drawFullScreenBackground(const ImU32 col) {
	// Fill current window (Scene Panel child)
	auto dl   = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size= ImGui::GetWindowSize();
	dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
	dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), col);
	dl->PopClipRect();
}

static void drawModelPlaceholder(Entity /*e*/, const char* label) {
	// Draw a centered textured quad (placeholder) inside the current Scene Panel window
	auto dl   = ImGui::GetWindowDrawList();
	ImVec2 pos  = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();
	dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);
	ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
	ImVec2 half   = ImVec2(48, 48);
	ImVec2 p0 = ImVec2(center.x - half.x, center.y - half.y);
	ImVec2 p1 = ImVec2(center.x + half.x, center.y + half.y);
	dl->AddImage(ResourceUtils::getPlaceholderTexture(), p0, p1, ImVec2(0,1), ImVec2(1,0));
	dl->AddRect(p0, p1, IM_COL32(200,220,255,255), 6.0f);
	// optional label
	size_t len = std::strlen(label);
	float offset = float(std::max<size_t>(4u, len)) * 3.5f;
 	dl->AddText(ImVec2(center.x - offset, center.y + half.y + 6.0f), IM_COL32(255,255,255,255), label);
	dl->PopClipRect();
}

static std::string normalizePath(const std::string& in);
static std::string sanitizeImagePath(std::string value);
static std::filesystem::path getProjectRoot();

static void drawUITextBox(const std::string& text) {
	// Draw a translucent box at bottom-center of current window with a single-line preview
	auto dl    = ImGui::GetWindowDrawList();
	ImVec2 pos  = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();
	dl->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);

	const float margin = 12.0f;
	float w = (std::min)(600.0f, size.x - 2.0f * margin - 56.0f);
	ImVec2 boxPos = ImVec2(pos.x + (size.x - w) * 0.5f, pos.y + size.y - 160.0f);
	// Clamp Y to keep the box fully inside the Scene Panel region
	float boxHeight = 120.0f;
	if (boxPos.y + boxHeight > pos.y + size.y - margin) boxPos.y = (pos.y + size.y - margin) - boxHeight;
	if (boxPos.y < pos.y + margin) boxPos.y = pos.y + margin;
	ImVec2 br     = ImVec2(boxPos.x + w, boxPos.y + 120.0f);

	dl->AddRectFilled(boxPos, br, IM_COL32(10, 10, 12, 200), 6.0f);
	dl->AddRect      (boxPos, br, IM_COL32(200,200,220,180), 4.0f);

	// Draw single line text (sufficient for MVP; wrapping can be added later)
	ImVec2 textPos = ImVec2(boxPos.x + 12.0f, boxPos.y + 10.0f);
	dl->AddText(textPos, IM_COL32(230,230,235,255), text.c_str());
	dl->PopClipRect();
}

// Texture cache for background images
static std::unordered_map<std::string, ImTextureID> s_bgTexCache;
static std::unordered_map<std::string, bool> s_bgTexLogState;
static std::unordered_map<std::string, std::filesystem::path> s_assetFilenameCache;
static Entity s_lastFlowNodeLogged = INVALID_ENTITY;
static std::vector<Entity> s_lastBackgroundList;
struct SceneLogEntry { std::string text; ImU32 color; };
static std::deque<SceneLogEntry> s_sceneDebugLog;
static std::string s_lastSceneLog;
static Entity s_lastBgEntity = INVALID_ENTITY;
static std::string s_lastBgImage;

static void pushSceneDebug(const std::string& msg, ImU32 color = IM_COL32(200,200,200,220)) {
	if (msg.empty() || msg == s_lastSceneLog) return;
	s_lastSceneLog = msg;
	s_sceneDebugLog.push_back({msg, color});
	while (s_sceneDebugLog.size() > 12) s_sceneDebugLog.pop_front();
}

static void logBackgroundLoad(const std::string& key, bool success, const std::string& resolvedPath) {
	auto it = s_bgTexLogState.find(key);
	if (it != s_bgTexLogState.end() && it->second == success) return;
	s_bgTexLogState[key] = success;
	if (success) {
		std::cout << "[RenderSystem] Loaded background '" << key << "' -> " << resolvedPath << "\n";
		pushSceneDebug("Loaded background '" + key + "'", IM_COL32(120,255,160,255));
	} else {
		std::cout << "[RenderSystem] FAILED background '" << key << "' (tried: " << resolvedPath << ")\n";
		pushSceneDebug("FAILED background '" + key + "' (verify path)", IM_COL32(255,140,140,255));
	}
}

// Public: allow invalidation if inspector changes the path
void invalidateTexture(const std::string& key) {
	const std::string normalizedKey = normalizePath(sanitizeImagePath(key));
	if (normalizedKey.empty()) return;
	s_bgTexCache.erase(normalizedKey);
	s_bgTexLogState.erase(normalizedKey);
}

static std::string normalizeSlashes(const std::string& in) {
	std::string out = in;
	std::replace(out.begin(), out.end(), '\\', '/');
	return out;
}

static bool startsWithInsensitive(const std::string& value, const std::string& prefix) {
	if (value.size() < prefix.size()) return false;
	for (size_t i = 0; i < prefix.size(); ++i) {
		if (std::tolower(static_cast<unsigned char>(value[i])) != std::tolower(static_cast<unsigned char>(prefix[i])))
			return false;
	}
	return true;
}

static std::string stripPrefixInsensitive(const std::string& value, const std::string& prefix) {
	if (!startsWithInsensitive(value, prefix)) return value;
	size_t idx = prefix.size();
	if (idx < value.size() && value[idx] == '/') ++idx;
	return value.substr(idx);
}

static std::string trimLeadingDotSlash(std::string value) {
	while (value.rfind("./", 0) == 0) value.erase(0, 2);
	return value;
}

static std::string sanitizeImagePath(std::string value) {
	auto isTrimChar = [](unsigned char c) {
		return std::isspace(c) || c == '\r' || c == '\n' || c == '\t';
	};
	while (!value.empty() && isTrimChar(static_cast<unsigned char>(value.front()))) value.erase(value.begin());
	while (!value.empty() && isTrimChar(static_cast<unsigned char>(value.back()))) value.pop_back();
	if (value.size() >= 2) {
		char first = value.front();
		char last  = value.back();
		if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) value = value.substr(1, value.size() - 2);
	}
	for (const char* prefix : {"file://", "res://", "asset://", "assets://"}) {
		if (startsWithInsensitive(value, prefix)) {
			value = value.substr(std::strlen(prefix));
			break;
		}
	}
	return trimLeadingDotSlash(value);
}

static std::filesystem::path detectProjectRootFallback() {
	std::filesystem::path dir = std::filesystem::current_path();
	for (int i = 0; i < 8 && !dir.empty(); ++i) {
		std::error_code ec;
		if (std::filesystem::exists(dir / "Runtime", ec) &&
		    std::filesystem::exists(dir / "Runtime" / "Assets", ec)) {
			return dir;
		}
		dir = dir.parent_path();
	}
	return {};
}

static std::string normalizePath(const std::string& in) {
	if (in.empty()) return {};
	std::string data = normalizeSlashes(std::filesystem::path(in).generic_string());
	data = trimLeadingDotSlash(data);
	std::string lower = data;
	std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	const std::string needle = "runtime/assets/";
	if (auto pos = lower.find(needle); pos != std::string::npos) {
		return data.substr(pos);
	}
	return data;
}

static std::filesystem::path getProjectRoot() {
	std::filesystem::path proj = ProjectManager::getCurrentProjectPath();
	if (!proj.empty()) {
		if (auto root = proj.parent_path(); !root.empty()) return root;
	}
	return detectProjectRootFallback();
}

static std::vector<std::filesystem::path> collectCandidateTexturePaths(const std::string& imagePath) {
	std::vector<std::filesystem::path> results;
	std::unordered_set<std::string> seen;
	auto pushPath = [&](const std::filesystem::path& path) {
		if (path.empty()) return;
		auto normalized = path.lexically_normal();
		std::string key = normalized.generic_string();
		if (key.empty()) return;
		if (seen.insert(key).second) results.push_back(normalized);
	};

	std::filesystem::path input(imagePath);
	if (input.is_absolute()) pushPath(input);

	const std::string normalized = trimLeadingDotSlash(normalizeSlashes(imagePath));
	std::vector<std::string> rels;
	auto pushRel = [&](const std::string& rel) {
		if (!rel.empty()) rels.push_back(rel);
	};
	pushRel(normalized);

	const std::string afterRuntime = trimLeadingDotSlash(stripPrefixInsensitive(normalized, "runtime"));
	pushRel(afterRuntime);
	pushRel("Runtime/" + afterRuntime);

	const std::string afterAssets = trimLeadingDotSlash(stripPrefixInsensitive(normalized, "assets"));
	pushRel(afterAssets);
	pushRel("Assets/" + afterAssets);
	pushRel("Runtime/Assets/" + afterAssets);

	const std::string afterRuntimeAssets = trimLeadingDotSlash(stripPrefixInsensitive(afterRuntime, "assets"));
	pushRel(afterRuntimeAssets);
	pushRel("Assets/" + afterRuntimeAssets);
	pushRel("Runtime/Assets/" + afterRuntimeAssets);

	const std::filesystem::path cwd = std::filesystem::current_path();
	const std::filesystem::path projectRoot = getProjectRoot();

	for (const auto& rel : rels) {
		std::filesystem::path relPath(rel);
		pushPath(cwd / relPath);
		if (!projectRoot.empty()) {
			pushPath(projectRoot / relPath);
			pushPath(projectRoot / "Runtime" / relPath);
			pushPath(projectRoot / "Runtime" / "Assets" / relPath);
			pushPath(projectRoot / "Assets" / relPath);
		}
	}

	return results;
}

static ImTextureID loadTextureFromFile(const std::string& absPath) {
	if (absPath.empty()) return (ImTextureID)0;
	// Use ResourceUtils to load as ImGui texture
	ImTextureID tex = ResourceUtils::loadTextureFromFile(absPath);
	return tex ? tex : (ImTextureID)0;
}

static std::filesystem::path findInRuntimeAssetsByName(const std::string& fileName) {
	if (fileName.empty()) return {};
	if (auto it = s_assetFilenameCache.find(fileName); it != s_assetFilenameCache.end())
		return it->second;

	auto projectRoot = getProjectRoot();
	if (projectRoot.empty()) return {};

	std::filesystem::path assets = projectRoot / "Runtime" / "Assets";
	std::error_code ec;
	if (!std::filesystem::exists(assets, ec)) return {};

	for (std::filesystem::recursive_directory_iterator it(assets, std::filesystem::directory_options::skip_permission_denied, ec);
	     it != std::filesystem::recursive_directory_iterator(); ++it) {
		if (!it->is_regular_file(ec)) continue;
		if (it->path().filename().string() != fileName) continue;
		auto canonical = std::filesystem::weakly_canonical(it->path(), ec);
		s_assetFilenameCache[fileName] = canonical.empty() ? it->path() : canonical;
		return s_assetFilenameCache[fileName];
	}

	s_assetFilenameCache[fileName] = std::filesystem::path();
	return {};
}

static ImTextureID getTextureForPath(const std::string& imagePath) {
	const std::string cleanedInput = sanitizeImagePath(imagePath);
	if (cleanedInput.empty()) return (ImTextureID)0;

	std::string lookupInput = ResourceManager::ingestBackgroundAsset(cleanedInput);
	if (lookupInput.empty()) lookupInput = cleanedInput;
	if (lookupInput != imagePath) {
		pushSceneDebug("Resolved background path: " + lookupInput,
		               IM_COL32(200,220,255,200));
	}

	// Use the same logic as renderAssetBrowser for resolving paths
	std::filesystem::path assetsRoot = ResourceManager::get().getAssetsRoot(); 
	std::filesystem::path resolvedPath = assetsRoot / lookupInput;

	if (!std::filesystem::exists(resolvedPath)) {
		std::cout << "[RenderSystem] Background path not found: " << resolvedPath << "\n";
		return ResourceUtils::getPlaceholderTexture();
	}

	ImTextureID tex = ResourceUtils::loadTextureFromFile(resolvedPath.generic_string());
	if (!tex) {
		std::cout << "[RenderSystem] Failed to load texture: " << resolvedPath << "\n";
		return ResourceUtils::getPlaceholderTexture();
	}

	return tex;
}

static void drawSceneDebugConsole() {
	if (s_sceneDebugLog.empty()) return;
	ImVec2 pos = ImGui::GetWindowPos();
	ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
	ImGui::SetNextWindowPos(ImVec2(pos.x + 12.0f, pos.y + 12.0f), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.6f);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
	                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
	if (ImGui::Begin("SceneDebugOverlay", nullptr, flags)) {
		for (const auto& entry : s_sceneDebugLog) {
			ImGui::PushStyleColor(ImGuiCol_Text, entry.color);
			ImGui::TextUnformatted(entry.text.c_str());
			ImGui::PopStyleColor();
		}
	}
	ImGui::End();
}

// Draw background for current scene (first background entity under FlowNode)
void renderCurrentSceneBackground() {
	auto& em = EntityManager::get();
	Entity node = SceneManager::get().getCurrentFlowNode();
	if (node == INVALID_ENTITY) return;

	auto fn = em.getComponent<FlowNodeComponent>(node);
	if (!fn) {
		pushSceneDebug("Current FlowNode missing FlowNodeComponent", IM_COL32(255,160,160,255));
		s_lastBgEntity = INVALID_ENTITY;
		s_lastBgImage.clear();
		return;
	}
	if (node != s_lastFlowNodeLogged || fn->backgroundEntities != s_lastBackgroundList) {
		std::ostringstream oss;
		oss << "FlowNode '" << fn->name << "' backgrounds:";
		if (fn->backgroundEntities.empty()) oss << " <none>";
		for (Entity b : fn->backgroundEntities) oss << " " << static_cast<unsigned>(b);
		pushSceneDebug(oss.str(), IM_COL32(180,220,255,255));

		for (Entity b : fn->backgroundEntities) {
			auto bgComp = em.getComponent<BackgroundComponent>(b);
			if (!bgComp) {
				pushSceneDebug(" - Entity " + std::to_string(static_cast<unsigned>(b)) + " missing BackgroundComponent",
				               IM_COL32(255,140,140,255));
			} else {
				std::string path = bgComp->image.empty() ? "<empty>" : bgComp->image;
				pushSceneDebug(" - Entity " + std::to_string(static_cast<unsigned>(b)) + " image: " + path,
				               IM_COL32(200,220,255,255));
			}
		}
		s_lastFlowNodeLogged = node;
		s_lastBackgroundList.assign(fn->backgroundEntities.begin(), fn->backgroundEntities.end());
	}
	if (fn->backgroundEntities.empty()) {
		pushSceneDebug("FlowNode '" + fn->name + "' has no background entities", IM_COL32(255,200,160,255));
		s_lastBgEntity = INVALID_ENTITY;
		s_lastBgImage.clear();
		return;
	}

	Entity bgEnt = fn->backgroundEntities.front();
	auto bg = em.getComponent<BackgroundComponent>(bgEnt);
	if (!bg) {
		pushSceneDebug("Entity " + std::to_string(static_cast<unsigned>(bgEnt)) + " missing BackgroundComponent", IM_COL32(255,160,160,255));
		return;
	}
	if (bgEnt != s_lastBgEntity) {
		pushSceneDebug("Using background entity " + std::to_string(static_cast<unsigned>(bgEnt)), IM_COL32(180,220,255,255));
		s_lastBgEntity = bgEnt;
	}
	if (bg->image != s_lastBgImage) {
		pushSceneDebug("Background image path: " + (bg->image.empty() ? std::string("<empty>") : bg->image), IM_COL32(200,220,255,255));
		s_lastBgImage = bg->image;
	}
	if (bg->image.empty()) {
		pushSceneDebug("Background image string is empty", IM_COL32(255,200,160,255));
	}

	ImTextureID tex = (ImTextureID)0;
	if (!bg->image.empty()) {
		tex = getTextureForPath(bg->image);
	}
	if (!tex) {
		tex = ResourceUtils::getPlaceholderTexture();
	}
	if (tex == ResourceUtils::getPlaceholderTexture()) {
		pushSceneDebug("Rendering placeholder texture (image failed to load)", IM_COL32(255,160,160,255));
	}
	drawTexturedBackground(tex, bgEnt);
}

// Integrate into your panel/frame render call (call this early before drawing other layers)
void renderScenePanel() {
	renderCurrentSceneBackground();
	drawSceneDebugConsole();
}

void beginScene() {
    // Clear screen, setup camera
}

void endScene() {
    // Present rendered frame
}

void renderEntityEditor(Entity e) {
	auto& em = EntityManager::get();
 	// BackgroundComponent -> full-screen background
 	if (auto bg = em.getComponent<BackgroundComponent>(e)) {
 		ImTextureID tex = (ImTextureID)0;
 		if (!bg->image.empty()) {
 			tex = getTextureForPath(bg->image);
 		}
 		if (!tex) tex = ResourceUtils::getPlaceholderTexture();
 		drawTexturedBackground(tex, e);
 		// optional: draw name/asset path
 		auto dl = ImGui::GetWindowDrawList();
 		ImVec2 wpos = ImGui::GetWindowPos();
 		dl->AddText(ImVec2(wpos.x + 8, wpos.y + 8), IM_COL32(220,220,220,255), "(background)");
 		return; // background covers large area; other layers are drawn separately by SceneManager order
 	}

 	// ModelComponent -> simple cube placeholder
 	if (auto mdl = em.getComponent<ModelComponent>(e)) {
 		drawModelPlaceholder(e, "Model");
 		return;
 	}

 	// Dialogue/Choice/Dice in editor: show simple text preview when selected as event
 	if (auto dlg = em.getComponent<DialogueComponent>(e)) {
 		std::string preview = dlg->lines.empty() ? "(no lines)" : dlg->lines.front();
 		drawUITextBox(preview);
 		return;
 	}
 	if (auto ch = em.getComponent<ChoiceComponent>(e)) {
 		std::string preview = ch->options.empty() ? "(no options)" : ch->options.front().text;
 		drawUITextBox(preview);
 		return;
 	}
 	if (auto dr = em.getComponent<DiceRollComponent>(e)) {
 		drawUITextBox(std::string("Dice d") + std::to_string(dr->sides) + " >= " + std::to_string(dr->threshold));
 		return;
 	}

	// Fallback: small label for unknown entity types
	auto dl = ImGui::GetForegroundDrawList();
	auto disp = ImGui::GetIO().DisplaySize;
	dl->AddText(ImVec2(8, disp.y - 22.0f), IM_COL32(200,200,200,180), ("Entity " + std::to_string((unsigned)e)).c_str());
}

void renderEntityRuntime(Entity e) {
	auto& em = EntityManager::get();

	// Runtime rendering mirrors editor placeholders for now
	// Background
	if (auto bg = em.getComponent<BackgroundComponent>(e)) {
		ImTextureID tex = (ImTextureID)0;
		if (!bg->image.empty()) {
			tex = getTextureForPath(bg->image);
		}
		if (!tex) tex = ResourceUtils::getPlaceholderTexture();
		drawTexturedBackground(tex, e);
		return;
	}
	// Model
	if (auto mdl = em.getComponent<ModelComponent>(e)) {
		// Model placeholder label
		drawModelPlaceholder(e, "Model");
		return;
	}

	// Helper: find FlowNode entity by name
	auto findNodeByName = [&](const std::string& nodeName) -> Entity {
		if (nodeName.empty()) return INVALID_ENTITY;
		Entity metaEntity = ProjectManager::getProjectMetaEntity();
		auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
		if (!base) return INVALID_ENTITY;
		auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);
		for (Entity n : meta->sceneNodes) {
			auto fn = em.getComponent<FlowNodeComponent>(n);
			if (fn && fn->name == nodeName) return n;
		}
		return INVALID_ENTITY;
	};

	// Helper: parse @Event:<id>
	auto parseEventTag = [&](const std::string& s) -> Entity {
		const std::string tag = "@Event:";
		if (s.rfind(tag, 0) == 0) {
			try {
				uint64_t id = std::stoull(s.substr(tag.size()));
				return (Entity)id;
			} catch (...) { return INVALID_ENTITY; }
		}
		return INVALID_ENTITY;
	};

	// Dialogue: show text and Continue button (interactive)
	if (auto dlg = em.getComponent<DialogueComponent>(e)) {
		// Draw preview box inside Scene Panel window (no extra floating windows)
		drawUITextBox(dlg->lines.empty() ? "(no lines)" : dlg->lines.front());

		// Small interactive overlay for the dialogue (buttons must be in an interactive window)
		// Anchor the control window inside the ScenePanel render region (center-bottom)
		{
			float rx = SceneManager::get().getRenderRegionX();
			float ry = SceneManager::get().getRenderRegionY();
			float rw = SceneManager::get().getRenderRegionW();
			float rh = SceneManager::get().getRenderRegionH();
			// Ensure the window is created on the same viewport as Scene Panel to prevent overlap with other editor panels
			ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
			ImVec2 winPos = ImVec2(rx + rw * 0.5f - 160.0f, ry + rh - 140.0f);
			ImGui::SetNextWindowPos(winPos, ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_Always);
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;
			ImGui::SetNextWindowBgAlpha(0.15f);
			if (ImGui::Begin("DialogueControls", nullptr, flags)) {
				if (!dlg->lines.empty()) {
					ImGui::TextWrapped("%s", dlg->lines.front().c_str());
				} else {
					ImGui::TextDisabled("(no lines)");
				}
				ImGui::Separator();
				if (dlg->advanceOnClick) {
					if (ImGui::Button("Continue")) {
						// Mark triggered and advance via FlowExecutor
						dlg->triggered = true;
						FlowExecutor::get().tick();
						// bind new scene if we ended the scene
						FlowExecutor::get().tick();
					}
				} else {
					ImGui::TextDisabled("Auto-advance disabled");
				}
				ImGui::End();
			}
		}
		return;
	}

	// Choice: present options and route on click
	if (auto ch = em.getComponent<ChoiceComponent>(e)) {
		// Draw preview inside Scene Panel window
		drawUITextBox(ch->options.empty() ? "(no options)" : ch->options.front().text);

		// Interactive choices (anchored)
		{
			float rx = SceneManager::get().getRenderRegionX();
			float ry = SceneManager::get().getRenderRegionY();
			float rw = SceneManager::get().getRenderRegionW();
			float rh = SceneManager::get().getRenderRegionH();
			ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
			ImVec2 pos = ImVec2(rx + rw * 0.5f - 180.0f, ry + rh - 160.0f);
			ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Always);
		}
		if (ImGui::Begin("ChoiceControls", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
			if (ch->options.empty()) {
				ImGui::TextDisabled("(no options)");
			} else {
				for (size_t i = 0; i < ch->options.size(); ++i) {
					const auto& opt = ch->options[i];
					// parse "Text -> Target"
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
 								try {
 									Entity ev = (Entity)std::stoull(target.substr(7));
 									if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(ev);
 								} catch (...) {}
 								FlowExecutor::get().tick(); // advance within scene
 							} else {
 								Entity dst = findNodeByName(target);
 								if (dst != INVALID_ENTITY) {
 									SceneManager::get().setCurrentFlowNode(dst);
 									if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(dst);
 									FlowExecutor::get().reset();
 									FlowExecutor::get().tick(); // bind new scene
 								} else {
 									// fallback: advance to next event
 									FlowExecutor::get().tick();
 								}
 							}
 						} else {
 							FlowExecutor::get().tick(); // default: next event
 						}
 					}
 				}
 			}
 			ImGui::End();
 		}
 		return;
 	}

	// DiceRoll: roll and route on success/failure
	if (auto dr = em.getComponent<DiceRollComponent>(e)) {
		// Draw preview inside Scene Panel window
		drawUITextBox(std::string("Dice d") + std::to_string(dr->sides) + " >= " + std::to_string(dr->threshold));

		// Anchor dice controls inside ScenePanel (bottom-center)
		{
			float rx = SceneManager::get().getRenderRegionX();
			float ry = SceneManager::get().getRenderRegionY();
			float rw = SceneManager::get().getRenderRegionW();
			float rh = SceneManager::get().getRenderRegionH();
			ImGui::SetNextWindowViewport(ImGui::GetWindowViewport()->ID);
			ImGui::SetNextWindowPos(ImVec2(rx + rw * 0.5f - 160.0f, ry + rh - 160.0f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_Always);
		}
		if (ImGui::Begin("DiceControls", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
			ImGui::Text("Roll d%d; success if >= %d", dr->sides, dr->threshold);
			if (ImGui::Button("Roll")) {
				int sides = (std::max)(1, dr->sides);
				std::random_device rd; std::mt19937 gen(rd());
				std::uniform_int_distribution<int> dist(1, sides);
				int roll = dist(gen);
				bool success = (roll >= dr->threshold);
				const std::string& nextName = success ? dr->onSuccess : dr->onFailure;

				// handle routing
				Entity evtTarget = parseEventTag(nextName);
				if (evtTarget != INVALID_ENTITY) {
					if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(evtTarget);
					FlowExecutor::get().tick(); // advance within scene
				} else if (!nextName.empty()) {
					Entity sceneTarget = findNodeByName(nextName);
					if (sceneTarget != INVALID_ENTITY) {
						SceneManager::get().setCurrentFlowNode(sceneTarget);
						if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(sceneTarget);
						FlowExecutor::get().reset();
						FlowExecutor::get().tick(); // bind new scene
					} else {
						FlowExecutor::get().tick(); // fallback next event
					}
				} else {
					FlowExecutor::get().tick(); // no target => next event
				}

				// Show a modal with simple result info
				ImGui::OpenPopup("Dice Result");
				if (ImGui::BeginPopupModal("Dice Result", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("You rolled: %d (%s)", roll, success ? "Success" : "Failure");
					if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
			}
			ImGui::End();
		}
		return;
	}

	// Fallback: label in current window bottom-left
	auto dl = ImGui::GetWindowDrawList();
	ImVec2 wpos = ImGui::GetWindowPos();
	ImVec2 wsize= ImGui::GetWindowSize();
	dl->AddText(ImVec2(wpos.x + 8, wpos.y + wsize.y - 22.0f), IM_COL32(200,200,200,180), ("Entity " + std::to_string((unsigned)e)).c_str());
}

} // namespace RenderSystem