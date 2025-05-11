#include "AssetRegistry.h"

#include <imgui.h>
#include <filesystem>
#include <algorithm>

using json = nlohmann::json;
using namespace AssetTypeRegistry;

std::unordered_map<AssetType, AssetRegistry::InspectorFunc> AssetRegistry::s_inspectorRegistry;
std::unordered_map<AssetType, std::vector<std::shared_ptr<AssetBase>>> AssetRegistry::m_assets;

std::vector<AssetType> AssetRegistry::getAllTypes() {
    std::vector<AssetType> out;
    for (const auto& info : AssetTypeRegistry::getAllTypes()) 
        out.push_back(info.type);
    return out;
}
void AssetRegistry::clear() {}

std::string AssetRegistry::getTypeKey(AssetType t) {
    if (const auto* info = getInfo(t)) return info->key;
    return "";
}

AssetType AssetRegistry::getTypeFromExtension(const std::string& path) {
    auto ext = std::filesystem::path(path).extension().string();
    if (const auto* info = getInfoByExtension(ext)) return info->type;
    return AssetType::Unknown;
}

std::vector<std::shared_ptr<AssetBase>> AssetRegistry::getAssets(AssetType type) {
    auto it = m_assets.find(type);
    if (it != m_assets.end()) return it->second;
    return {};
}

bool AssetRegistry::importFile(const std::string& path) {
    auto ext = std::filesystem::path(path).extension().string();
    if (const auto* info = getInfoByExtension(ext)) return info->fileImporter(path);
    return false;
}

std::vector<json> AssetRegistry::getJsonArrayForType(AssetType type, const json& j) {
    if (const auto* info = getInfo(type))
        if (j.contains(info->key) && j[info->key].is_array())
            return j[info->key].get<std::vector<json>>();
    return {};
}

bool AssetRegistry::loadFromJsonArray(AssetType type, const json& e) {
    if (const auto* info = getInfo(type)) return info->fileImporter(""); // stub
    return false;
}

void AssetRegistry::importAllFromJson(const json& j) {
    for (auto t : getAllTypes())
        for (const auto& e : getJsonArrayForType(t, j))
            loadFromJsonArray(t, e);
}

void AssetRegistry::registerInspector(AssetType type, InspectorFunc func) {
    s_inspectorRegistry[type] = std::move(func);
}

void AssetRegistry::renderInspector(AssetType type, const std::string& id) {
    auto it = s_inspectorRegistry.find(type);
    if (it != s_inspectorRegistry.end()) {
        it->second(id);
    } else {
        ImGui::Text("Unsupported asset type.");
    }
}

void AssetRegistry::add(const std::shared_ptr<AssetBase>& asset) {
    m_assets[asset->getType()].push_back(asset);
}