#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Assets/Character.h"
#include "Assets/TextAsset.h"
#include "Assets/AudioAsset.h"

class ResourceManager {
public:
    static ResourceManager& get();
    void clear();

    void addCharacter(std::shared_ptr<Character> character);
    const std::vector<std::shared_ptr<Character>>& getCharacters() const;

    // Text
    void addText(std::shared_ptr<TextAsset> text);
    const std::vector<std::shared_ptr<TextAsset>>& getTexts() const;
    
    // Audio
    void addAudio(std::shared_ptr<AudioAsset> audio);
    const std::vector<std::shared_ptr<AudioAsset>>& getAudios() const;

private:
    std::vector<std::shared_ptr<Character>> m_characters;
    std::vector<std::shared_ptr<TextAsset>> m_textAssets;
    std::vector<std::shared_ptr<AudioAsset>> m_audioAssets;
};
