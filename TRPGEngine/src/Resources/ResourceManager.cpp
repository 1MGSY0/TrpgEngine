#include "ResourceManager.hpp"
#include "UI/EditorUI.hpp"
#include "Project/ProjectManager.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

// -------------------------------
// Singleton Instance
// -------------------------------
ResourceManager& ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

// -------------------------------
// In-Memory Management
// -------------------------------
void ResourceManager::clear() {
    m_unsaved = false;
}

bool ResourceManager::hasUnsavedChanges() const {
    return m_unsaved;
}

void ResourceManager::setUnsavedChanges(bool value) {
    m_unsaved = value;
}

// -------------------------------
// Asset File Handling
// -------------------------------
std::optional<nlohmann::json> ResourceManager::loadAssetFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return std::nullopt;

    nlohmann::json j;
    in >> j;

    return j;
}

bool ResourceManager::saveAssetFile(const nlohmann::json& j, const std::string& id, const std::string& extension) {
    auto* editor = EditorUI::get();
    if (!editor) return false;

    const auto& folder = editor->getSelectedFolder();
    std::filesystem::create_directories(folder);

    std::string fileName = id + extension;
    std::filesystem::path path = folder / fileName;

    std::ofstream out(path);
    std::cout << "Saving to: " << path.string() << std::endl;
    if (!out.is_open()) {
        std::cout << "[ERROR] Failed to open file for writing.\n";
        return false;
    }

    out << j.dump(4);
    m_unsaved = true;

    editor->forceFolderRefresh();
    return true;
}

bool ResourceManager::renameAssetFile(const std::string& oldPath, const std::string& newName) {
    std::filesystem::path oldP(oldPath);
    std::filesystem::path newP = oldP.parent_path() / newName;

    std::error_code ec;
    std::filesystem::rename(oldP, newP, ec);
    
    if (!ec && EditorUI::get()) {
        EditorUI::get()->forceFolderRefresh();
    }

    return !ec;
}

bool ResourceManager::deleteAssetFile(const std::string& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);

    if (!ec && EditorUI::get()) {
        EditorUI::get()->forceFolderRefresh();
    }

    return !ec;
}

void ResourceManager::refreshFolder() {
    if (auto* editor = EditorUI::get()) {
        editor->forceFolderRefresh();
    }
}

bool ResourceManager::importFileAsset(const std::string& sourcePath, FileAssetType type) {
    if (!std::filesystem::exists(sourcePath)) return false;

    std::filesystem::path source = sourcePath;
    std::string extension = source.extension().string();
    std::string fileName = source.filename().string();

    // Define base folder for asset type
    std::filesystem::path targetFolder = "Runtime/Assets";

    std::filesystem::create_directories(targetFolder);
    std::filesystem::path targetPath = targetFolder / fileName;

    // Avoid overwriting: add suffix if needed
    int suffix = 1;
    while (std::filesystem::exists(targetPath)) {
        targetPath = targetFolder / (source.stem().string() + "_" + std::to_string(suffix++) + extension);
    }

    std::error_code ec;
    std::filesystem::copy_file(source, targetPath, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) return false;

    if (auto* editor = EditorUI::get()) {
        editor->forceFolderRefresh();
    }

    return true;
}

namespace {
std::filesystem::path resolveProjectRoot() {
    std::string projStr = ProjectManager::getCurrentProjectPath();
    std::filesystem::path proj = projStr;
    std::error_code ec;

    // 1) If project path is configured, use that.
    if (!proj.empty()) {
        if (!proj.is_absolute())
            proj = std::filesystem::current_path() / proj;

        proj = std::filesystem::weakly_canonical(proj, ec);
        if (!ec) {
            std::filesystem::path root = proj.parent_path();
            if (!root.empty()) {
                return root;
            }
        }
    }
    std::filesystem::path dir = std::filesystem::current_path();
    std::filesystem::path best;

    for (int i = 0; i < 8 && !dir.empty(); ++i) {
        if (std::filesystem::exists(dir / "Runtime", ec) &&
            std::filesystem::exists(dir / "Runtime" / "Assets", ec)) {
            best = dir; // keep updating; last one is highest
        }
        dir = dir.parent_path();
    }

    return best;
}
} // namespace

std::string ResourceManager::ingestBackgroundAsset(const std::string& sourcePath) {
	if (sourcePath.empty()) return {};
	std::filesystem::path src(sourcePath);
	std::filesystem::path projectRoot = resolveProjectRoot();
	if (projectRoot.empty()) return sourcePath;

	std::error_code ec;
	std::filesystem::path backgroundsDir = projectRoot / "Runtime" / "Assets" / "Backgrounds";

	if (!src.is_absolute()) {
		std::filesystem::path abs = projectRoot / src;
		if (std::filesystem::exists(abs, ec)) {
			return src.lexically_normal().generic_string();
		}
		std::filesystem::path fallback = backgroundsDir / src.filename();
		if (std::filesystem::exists(fallback, ec)) {
			auto rel = std::filesystem::relative(fallback, projectRoot, ec);
			if (!ec) return rel.lexically_normal().generic_string();
		}
		return src.generic_string();
	}

	if (!std::filesystem::exists(src, ec) || !std::filesystem::is_regular_file(src, ec)) return sourcePath;

	std::string ext = src.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp") return sourcePath;

	std::filesystem::create_directories(backgroundsDir, ec);
	if (ec) return sourcePath;

	std::filesystem::path dest = backgroundsDir / src.filename();
	if (!std::filesystem::equivalent(src, dest, ec)) {
		std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing, ec);
		if (ec) return sourcePath;
	}

	auto relative = std::filesystem::relative(dest, projectRoot, ec);
	if (ec) return sourcePath;

	return relative.lexically_normal().generic_string();
}

std::filesystem::path ResourceManager::getAssetsRoot() const {
    std::error_code ec;
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path assetsRoot = cwd;

    assetsRoot = std::filesystem::weakly_canonical(assetsRoot, ec);
    std::cout << "[ResourceManager] Resolved assetsRoot: " << assetsRoot << "\n";

    return assetsRoot;
}


