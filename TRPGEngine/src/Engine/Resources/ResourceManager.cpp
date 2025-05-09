#include "Engine/Resources/ResourceManager.h"
#include "Engine/Assets/AssetImporter.h"
#include "Engine/Entity/Entity.h"
#include "UI/IPanel.h"

#include <fstream>
#include <filesystem>
#include <json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

ResourceManager& ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

// --- TEXT ---
void ResourceManager::addText(const std::string& id, std::shared_ptr<TextComponent> text) {
    m_textAssets.push_back(text);
    m_textMap[id] = text;
    setUnsavedChanges(true);
}

std::shared_ptr<TextComponent> ResourceManager::getText(const std::string& id) {
    if (m_textMap.count(id)) return m_textMap.at(id);
    return nullptr;
}

const std::vector<std::shared_ptr<TextComponent>>& ResourceManager::getAllTexts() const {
    return m_textAssets;
}

// --- CHARACTER ---
void ResourceManager::addCharacter(const std::string& id, std::shared_ptr<CharacterComponent> c) {
    m_characterAssets.push_back(c);
    m_characterMap[id] = c;
    setUnsavedChanges(true);
}

std::shared_ptr<CharacterComponent> ResourceManager::getCharacter(const std::string& id) {
    if (m_characterMap.count(id)) return m_characterMap.at(id);
    return nullptr;
}

const std::vector<std::shared_ptr<CharacterComponent>>& ResourceManager::getAllCharacters() const {
    return m_characterAssets;
}

// --- AUDIO ---
void ResourceManager::addAudio(const std::string& id, std::shared_ptr<AudioComponent> a) {
    m_audioAssets.push_back(a);
    m_audioMap[id] = a;
    setUnsavedChanges(true);
}

std::shared_ptr<AudioComponent> ResourceManager::getAudio(const std::string& id) {
    if (m_audioMap.count(id)) return m_audioMap.at(id);
    return nullptr;
}

const std::vector<std::shared_ptr<AudioComponent>>& ResourceManager::getAllAudio() const {
    return m_audioAssets;
}

// --- IMPORT ---
bool ResourceManager::importAssetFromFile(const std::string& filePath, AssetType type) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    json j;
    file >> j;
    file.close();

    std::string defaultId = fs::path(filePath).stem().string();  // fallback if no ID in JSON

    switch (type) {
        case AssetType::Text: {
            auto asset = std::make_shared<TextComponent>();
            asset->fromJson(j);
            std::string id = asset->getId().empty() ? defaultId : asset->getId();
            addText(id, asset);
            break;
        }
        case AssetType::Character: {
            auto asset = std::make_shared<CharacterComponent>();
            asset->fromJson(j);
            std::string id = asset->getId().empty() ? defaultId : asset->getId();
            addCharacter(id, asset);
            break;
        }
        case AssetType::Audio: {
            auto asset = std::make_shared<AudioComponent>();
            asset->fromJson(j);
            std::string id = asset->getId().empty() ? defaultId : asset->getId();
            addAudio(id, asset);
            break;
        }
        default:
            return false;
    }

    setUnsavedChanges(true);
    return true;
}

// --- CLEAR ---
void ResourceManager::clear() {
    m_textAssets.clear();
    m_characterAssets.clear();
    m_audioAssets.clear();

    m_textMap.clear();
    m_characterMap.clear();
    m_audioMap.clear();

    setUnsavedChanges(false);
}

// --- STATE ---
bool ResourceManager::hasUnsavedChanges() const {
    return m_unsavedChanges;
}

void ResourceManager::setUnsavedChanges(bool changed) {
    m_unsavedChanges = changed;
}