#pragma once
#include <string>
#include <vector>

enum class AssetType;

class ImportManager {
public:
    static bool importAsset(const std::string& filePath, AssetType type);
    static bool importMultiple(const std::vector<std::string>& filePaths, AssetType type);
};
