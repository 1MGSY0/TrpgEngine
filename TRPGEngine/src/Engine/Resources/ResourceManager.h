#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include "Engine/Entity/Components/TextComponent.h"
#include "Engine/Entity/Components/CharacterComponent.h"
#include "Engine/Entity/Components/AudioComponent.h"

// Enum for distinguishing asset import type
enum class AssetType {
    Text,
    Character,
    Audio
};


class ResourceManager {
public:

    static ResourceManager& get();

    // TEXT
    void addText(const std::string& id, std::shared_ptr<TextComponent> text);
    std::shared_ptr<TextComponent> getText(const std::string& id);
    const std::vector<std::shared_ptr<TextComponent>>& getAllTexts() const;

    // CHARACTER
    void addCharacter(const std::string& id, std::shared_ptr<CharacterComponent> c);
    std::shared_ptr<CharacterComponent> getCharacter(const std::string& id);
    const std::vector<std::shared_ptr<CharacterComponent>>& getAllCharacters() const;

    // AUDIO
    void addAudio(const std::string& id, std::shared_ptr<AudioComponent> a);
    std::shared_ptr<AudioComponent> getAudio(const std::string& id);
    const std::vector<std::shared_ptr<AudioComponent>>& getAllAudio() const;

    // --- Import from JSON file ---
    bool importAssetFromFile(const std::string& filePath, AssetType type);

    void clear();
    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool changed);

private:
    ResourceManager() = default;

    std::unordered_map<std::string, std::shared_ptr<TextComponent>> m_textMap;
    std::unordered_map<std::string, std::shared_ptr<CharacterComponent>> m_characterMap;
    std::unordered_map<std::string, std::shared_ptr<AudioComponent>> m_audioMap;

    std::vector<std::shared_ptr<TextComponent>> m_textAssets;
    std::vector<std::shared_ptr<CharacterComponent>> m_characterAssets;
    std::vector<std::shared_ptr<AudioComponent>> m_audioAssets;

    bool m_unsavedChanges = false;
};


