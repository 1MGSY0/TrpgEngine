#pragma once
#include <string>
#include <filesystem>
#include "Engine/Assets/AssetBase.h"
#include "Engine/Assets/AssetRegistry.h"

class ResourceManager {
public:
    static ResourceManager& get();

    bool importAssetFromFile(const std::string& path);
    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool v);
    void clear();

    std::vector<std::shared_ptr<AssetBase>> getAssets(AssetType type);

    template <typename T>
    void addAsset(const std::shared_ptr<T>& asset) {
        static_assert(std::is_base_of<AssetBase, T>::value, "Must inherit from AssetBase");
        m_registry.add(asset);
    }
    
private:
    ResourceManager() = default;
    AssetRegistry m_registry;
    bool m_unsaved = false;
};
