#include "ResourceManager.h"
#include "UI/EditorUI.h"
#include "Engine/Entity/ComponentRegistry.h"
#include "Engine/Entity/ComponentType.h"
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
// In-Memory Component Management
// -------------------------------
void ResourceManager::clear() {
    m_registry.clear();
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
    if (!out.is_open()) return false;

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
    std::filesystem::path targetFolder = "Assets";

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



// -------------------------------
// Component File Handling (Convert Between JSON and Registry)
// -------------------------------
bool ResourceManager::importComponentFromJson(const json& j, ComponentType type) {
    auto component = ComponentTypeRegistry::deserializeComponent(type, j);
    if (!component) return false;

    m_registry.add(component);
    m_unsaved = true;
    return true;
}

bool ResourceManager::exportComponentToJson(const std::shared_ptr<ComponentBase>& component, json& outJson) {
    if (!component) return false;
    outJson = component->toJson();
    return true;
}

bool ResourceManager::loadComponentsFromJsonArray(ComponentType type, const nlohmann::json& jArray) {
    const auto* info = ComponentTypeRegistry::getInfo(type);
    if (!info || !info->loader) return false;

    for (const auto& j : jArray) {
        auto component = std::static_pointer_cast<ComponentBase>(info->loader(j));
        if (component) {
            m_registry.add(component);
            m_unsaved = true;
        }
    }
    return true;
}

void ResourceManager::importAllComponentsFromJson(const nlohmann::json& j) {
    for (const auto& info : ComponentTypeRegistry::getAllInfos()) {
        if (j.contains(info.key)) {
            loadComponentsFromJsonArray(info.type, j[info.key]);
        }
    }
}

// -------------------------------
// Component Handling
// -------------------------------

void ResourceManager::addComponent(const std::shared_ptr<ComponentBase>& component) {
    m_registry.add(component);
    m_unsaved = true;
}

bool ResourceManager::saveComponentToAssetFile(const std::shared_ptr<ComponentBase>& component) {
    auto* editor = EditorUI::get();
    if (!editor) return false;

    const auto& folder = editor->getSelectedFolder();
    nlohmann::json j = component->toJson();
    std::string extension = ComponentTypeRegistry::getDefaultExtension(component->getType());

    return saveAssetFile(j, component->getID(), extension);
}

void ResourceManager::editComponent(const std::shared_ptr<ComponentBase>& component) {
    if (!component) return;
    m_unsaved = true;
}

bool ResourceManager::deleteComponent(const std::shared_ptr<ComponentBase>& component) {
    if (!component) return false;

    ComponentType type = component->getType();
    auto& components = m_registry.getMutableComponents(type);  // you’ll need to expose a mutable getter

    auto it = std::remove_if(components.begin(), components.end(),
        [&](const std::shared_ptr<ComponentBase>& c) {
            return c->getID() == component->getID();
        });

    if (it != components.end()) {
        components.erase(it, components.end());
        m_unsaved = true;
        return true;
    }

    return false;
}

std::shared_ptr<ComponentBase> ResourceManager::createComponent(ComponentType type, const std::string& defaultName) {
    const auto* info = ComponentTypeRegistry::getInfo(type);
    if (!info || !info->factory) return nullptr;

    auto component = info->factory();
    if (!component) return nullptr;

    addComponent(component);

    // Save to selected folder
    auto* editor = EditorUI::get();
    if (editor) {
        saveComponentToAssetFile(component); // uses selected folder now
    }

    return component;
}

