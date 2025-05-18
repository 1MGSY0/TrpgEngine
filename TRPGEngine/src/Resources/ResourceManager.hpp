#pragma once

#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "FileAssetType.hpp" 

class ResourceManager {
public:
    static ResourceManager& get();

    void clear();
    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool value);

    std::optional<nlohmann::json> loadAssetFile(const std::string& path);
    bool saveAssetFile(const nlohmann::json& j, const std::string& id, const std::string& extension);
    bool renameAssetFile(const std::string& oldPath, const std::string& newName);
    bool deleteAssetFile(const std::string& path);
    bool importFileAsset(const std::string& sourcePath, FileAssetType type);

    void refreshFolder();

private:
    ResourceManager() = default;
    bool m_unsaved = false;
};