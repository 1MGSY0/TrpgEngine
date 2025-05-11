#pragma once
#include "AssetBase.h"
#include "AssetType.h"
#include <string>
#include <vector>
#include <json.hpp>

class AssetRegistry {
public:
    void clear();
    void add(const std::shared_ptr<AssetBase>& asset);

    static std::vector<AssetType> getAllTypes();
    static std::string getTypeKey(AssetType type);
    static AssetType getTypeFromExtension(const std::string& path);
    
    std::vector<std::shared_ptr<AssetBase>> getAssets(AssetType type);

    static bool importFile(const std::string& path);

    static std::vector<nlohmann::json> getJsonArrayForType(AssetType type, const nlohmann::json& j);
    static bool loadFromJsonArray(AssetType type, const nlohmann::json& e);
    static void importAllFromJson(const nlohmann::json& j);

    static void renderInspector(AssetType type, const std::string& id);

private:
    using InspectorFunc = std::function<void(const std::string&)>;
    static std::unordered_map<AssetType, InspectorFunc> s_inspectorRegistry;
    static std::unordered_map<AssetType, std::vector<std::shared_ptr<AssetBase>>> m_assets;

public:
    static void registerInspector(AssetType type, InspectorFunc func);
};
