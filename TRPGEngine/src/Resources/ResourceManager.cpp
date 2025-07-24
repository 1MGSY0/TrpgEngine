#include "ResourceManager.hpp"
#include "UI/EditorUI.hpp"
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


