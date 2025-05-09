#include "ImportManager.h"
#include "Engine/Resources/ResourceManager.h"
#include <iostream>

bool ImportManager::importAsset(const std::string& filePath, AssetType type) {
    return ResourceManager::get().importAssetFromFile(filePath, type);
}

bool ImportManager::importMultiple(const std::vector<std::string>& filePaths, AssetType type) {
    bool allSuccess = true;
    for (const auto& path : filePaths) {
        if (!importAsset(path, type)) {
            std::cerr << "[ImportManager] Failed to import: " << path << "\n";
            allSuccess = false;
        }
    }
    return allSuccess;
}
