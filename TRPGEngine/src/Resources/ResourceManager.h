#pragma once

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Engine/Entity/ComponentBase.h"
#include "Engine/Entity/ComponentType.h"
#include "Engine/Entity/ComponentRegistry.h"
#include "FileAssetType.h" 

class ResourceManager {
public:
    static ResourceManager& get();

    // -------------------------------
    // In-Memory Component Management
    // -------------------------------
    void clear();
    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool value);

    const std::vector<std::shared_ptr<ComponentBase>> getAssets(ComponentType type);
    const std::unordered_map<ComponentType, std::vector<std::shared_ptr<ComponentBase>>>& getAllAssets() const;

    // -------------------------------
    // Asset File Handling (Generic I/O)
    // -------------------------------
    std::optional<nlohmann::json> loadAssetFile(const std::string& path);
    bool saveAssetFile(const nlohmann::json& j, const std::string& id, const std::string& extension);
    bool renameAssetFile(const std::string& oldPath, const std::string& newName);
    bool deleteAssetFile(const std::string& path);
    void refreshFolder();

    // -------------------------------
    // Component File Handling (Import / Export)
    // -------------------------------
    bool importComponentFromJson(const nlohmann::json& j, ComponentType type);
    bool loadComponentsFromJsonArray(ComponentType type, const nlohmann::json& jArray);
    void importAllComponentsFromJson(const nlohmann::json& j);
    bool exportComponentToJson(const std::shared_ptr<ComponentBase>& component, nlohmann::json& outJson);
    bool importFileAsset(const std::string& sourcePath, FileAssetType type);

    // -------------------------------
    // Generic Component Asset Creation
    // -------------------------------
    void addComponent(const std::shared_ptr<ComponentBase>& component);
    bool saveComponentToAssetFile(const std::shared_ptr<ComponentBase>& component);
    void editComponent(const std::shared_ptr<ComponentBase>& component);
    bool deleteComponent(const std::shared_ptr<ComponentBase>& component);
    std::shared_ptr<ComponentBase> createComponent(ComponentType type, const std::string& defaultName = "NewAsset");

private:
    ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

private:
    ComponentRegistry m_registry;
    bool m_unsaved = false;
};