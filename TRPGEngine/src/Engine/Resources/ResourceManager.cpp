
#include "ResourceManager.h"

ResourceManager& ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

std::vector<std::shared_ptr<AssetBase>> ResourceManager::getAssets(AssetType type) {
    return m_registry.getAssets(type);
}

bool ResourceManager::importAssetFromFile(const std::string& path) {
    if (!AssetRegistry::importFile(path)) return false;
    m_unsaved = true;
    return true;
}

bool ResourceManager::hasUnsavedChanges() const {
    return m_unsaved;
}

void ResourceManager::setUnsavedChanges(bool v) {
    m_unsaved = v;
}

void ResourceManager::clear() {
    m_registry.clear();
    m_unsaved = false;
}