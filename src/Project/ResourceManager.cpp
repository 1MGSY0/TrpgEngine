#include "ResourceManager.h"

ResourceManager& ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::addCharacter(std::shared_ptr<Character> character) {
    m_characters.push_back(character);
}

const std::vector<std::shared_ptr<Character>>& ResourceManager::getCharacters() const {
    return m_characters;
}

void ResourceManager::addText(std::shared_ptr<TextAsset> text) {
    m_textAssets.push_back(text);
}
const std::vector<std::shared_ptr<TextAsset>>& ResourceManager::getTexts() const {
    return m_textAssets;
}

void ResourceManager::addAudio(std::shared_ptr<AudioAsset> audio) {
    m_audioAssets.push_back(audio);
}
const std::vector<std::shared_ptr<AudioAsset>>& ResourceManager::getAudios() const {
    return m_audioAssets;
}

void ResourceManager::clear() {
    m_characters.clear();
    m_textAssets.clear();
    m_audioAssets.clear();
}