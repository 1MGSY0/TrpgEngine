#pragma once

#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/BackgroundComponent.hpp"
#include "Resources/ResourceManager.hpp"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <memory>

#ifdef _WIN32
#include <windows.h>
static bool OpenImageFileDialogWin32(std::string& outPath) {
	// Allow png/jpg/jpeg
	char szFile[MAX_PATH] = {0};
	OPENFILENAMEA ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = "Images (*.png;*.jpg;*.jpeg)\0*.png;*.jpg;*.jpeg\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn)) {
		outPath = szFile;
		return true;
	}
	return false;
}
#endif

static std::string NormalizeToProjectRelative(const std::string& absPath) {
	// Convert to "Runtime/Assets/..." when possible, otherwise keep as-is (absolute)
	std::filesystem::path p(absPath);
	std::string s = std::filesystem::weakly_canonical(p).generic_string();
	const std::string key1 = "Runtime/Assets/";
	const std::string key2 = "Runtime\\Assets\\";
	size_t pos = s.find(key1);
	if (pos != std::string::npos) return s.substr(pos);
	pos = s.find(key2);
	if (pos != std::string::npos) {
		std::string norm = s.substr(pos);
		for (char& c : norm) if (c == '\\') c = '/';
		return norm;
	}
	return s;
}

// Forward: invalidate BG texture cache when path changes
namespace RenderSystem { void invalidateTexture(const std::string& key); }

inline void RenderBackgroundPanel(Entity e) {
	auto& em = EntityManager::get();
	auto comp = em.getComponent<BackgroundComponent>(e);
	if (!comp) {
		ImGui::TextDisabled("No BackgroundComponent");
		return;
	}

	ImGui::SeparatorText("Background");

	// Path display (read-only)
	ImGui::InputText("Image Path", (char*)comp->image.c_str(), (int)comp->image.size(), ImGuiInputTextFlags_ReadOnly);

	// Actions: Browse / Clear
	if (ImGui::Button("Browse...")) {
#ifdef _WIN32
		std::string sel;
		if (OpenImageFileDialogWin32(sel)) {
			// Invalidate previous (if any)
			RenderSystem::invalidateTexture(comp->image);
			comp->image = NormalizeToProjectRelative(sel);
			RenderSystem::invalidateTexture(comp->image); // ensure fresh load
			ResourceManager::get().setUnsavedChanges(true);
		}
#else
		// TODO: Hook to your cross-platform file dialog
#endif
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		RenderSystem::invalidateTexture(comp->image);
		comp->image.clear();
		ResourceManager::get().setUnsavedChanges(true);
	}

	// Drag-n-drop target: accept asset payloads and OS file drops
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 56));
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::TextDisabled("Drop image here (.png/.jpg)");
	ImGui::InvisibleButton("##bg_drop_zone", ImVec2(-FLT_MIN, 48));
	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = nullptr;

		// Internal asset browser payload
		payload = ImGui::AcceptDragDropPayload("ASSET_PATH");
		if (payload && payload->Data) {
			const char* rel = static_cast<const char*>(payload->Data);
			if (rel && *rel) {
				RenderSystem::invalidateTexture(comp->image);
				comp->image = NormalizeToProjectRelative(rel);
				RenderSystem::invalidateTexture(comp->image);
				ResourceManager::get().setUnsavedChanges(true);
			}
		} else {
			// Common OS/file payload fallbacks (backend-dependent)
			if ((payload = ImGui::AcceptDragDropPayload("FILE_PATH")) && payload->Data) {
				const char* abs = static_cast<const char*>(payload->Data);
				if (abs && *abs) {
					RenderSystem::invalidateTexture(comp->image);
					comp->image = NormalizeToProjectRelative(abs);
					RenderSystem::invalidateTexture(comp->image);
					ResourceManager::get().setUnsavedChanges(true);
				}
			} else if ((payload = ImGui::AcceptDragDropPayload("OS_FILE_PATHS")) && payload->Data) {
				const char* abs = static_cast<const char*>(payload->Data);
				if (abs && *abs) {
					RenderSystem::invalidateTexture(comp->image);
					comp->image = NormalizeToProjectRelative(abs);
					RenderSystem::invalidateTexture(comp->image);
					ResourceManager::get().setUnsavedChanges(true);
				}
			} else if ((payload = ImGui::AcceptDragDropPayload("TEXT")) && payload->Data) {
				const char* abs = static_cast<const char*>(payload->Data);
				if (abs && *abs) {
					RenderSystem::invalidateTexture(comp->image);
					comp->image = NormalizeToProjectRelative(abs);
					RenderSystem::invalidateTexture(comp->image);
					ResourceManager::get().setUnsavedChanges(true);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::EndGroup();
}

// Inspector entry point expected by the Component system
// Matches: void renderBackgroundInspector(const std::shared_ptr<BackgroundComponent>&)
inline void renderBackgroundInspector(const std::shared_ptr<BackgroundComponent>& comp) {
	if (!comp) {
		ImGui::TextDisabled("No BackgroundComponent");
		return;
	}

	ImGui::SeparatorText("Background");

	// Display current path (read-only)
	ImGui::TextDisabled("Image Path");
	ImGui::SameLine();
	ImGui::TextWrapped("%s", comp->image.c_str());

	// Actions: Browse / Clear
	if (ImGui::Button("Browse...")) {
#ifdef _WIN32
		std::string sel;
		if (OpenImageFileDialogWin32(sel)) {
			comp->image = NormalizeToProjectRelative(sel);
			ResourceManager::get().setUnsavedChanges(true);
		}
#else
		// TODO: Hook to your cross-platform file dialog
#endif
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		comp->image.clear();
		ResourceManager::get().setUnsavedChanges(true);
	}

	// Drag-n-drop target: accept internal asset payloads and OS file drops
	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 56));
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::TextDisabled("Drop image here (.png/.jpg/.jpeg)");
	ImGui::InvisibleButton("##bg_drop_zone_shared", ImVec2(-FLT_MIN, 48));
	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload* payload = nullptr;

		// Internal asset browser payload
		payload = ImGui::AcceptDragDropPayload("ASSET_PATH");
		if (payload && payload->Data) {
			const char* rel = static_cast<const char*>(payload->Data);
			if (rel && *rel) {
				comp->image = NormalizeToProjectRelative(rel);
				ResourceManager::get().setUnsavedChanges(true);
			}
		} else {
			// Common OS/file payload fallbacks (backend-dependent)
			if ((payload = ImGui::AcceptDragDropPayload("FILE_PATH")) && payload->Data) {
				const char* abs = static_cast<const char*>(payload->Data);
				if (abs && *abs) {
					comp->image = NormalizeToProjectRelative(abs);
					ResourceManager::get().setUnsavedChanges(true);
				}
			} else if ((payload = ImGui::AcceptDragDropPayload("OS_FILE_PATHS")) && payload->Data) {
				const char* abs = static_cast<const char*>(payload->Data);
				if (abs && *abs) {
					comp->image = NormalizeToProjectRelative(abs);
					ResourceManager::get().setUnsavedChanges(true);
				}
			} else if ((payload = ImGui::AcceptDragDropPayload("TEXT")) && payload->Data) {
				const char* abs = static_cast<const char*>(payload->Data);
				if (abs && *abs) {
					comp->image = NormalizeToProjectRelative(abs);
					ResourceManager::get().setUnsavedChanges(true);
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::EndGroup();
}