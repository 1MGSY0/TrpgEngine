#pragma once
#include <string>
#include <vector>
#include <functional>

enum class AssetType {
    Unknown = 0,
    Text,
    Image,
    Audio,
    Video,
    Character,
    Script,
};


struct AssetTypeInfo {
    AssetType type;
    std::string key;
    std::vector<std::string> extensions;
    std::function<bool(const std::string&)> fileImporter;
};

namespace AssetTypeRegistry {
    const AssetTypeInfo* getInfo(AssetType type);
    const AssetTypeInfo* getInfoByExtension(const std::string& ext);
    const std::vector<AssetTypeInfo>& getAllTypes();
}