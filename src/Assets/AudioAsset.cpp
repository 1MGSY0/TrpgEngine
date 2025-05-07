#include "AudioAsset.h"

AudioAsset::AudioAsset(const std::string& name, const std::string& filePath)
    : m_name(name), m_filePath(filePath) {}

std::string AudioAsset::getName() const { return m_name; }
std::string AudioAsset::getFilePath() const { return m_filePath; }

json AudioAsset::toJson() const {
    return json{{"name", m_name}, {"path", m_filePath}};
}

std::shared_ptr<AudioAsset> AudioAsset::fromJson(const json& j) {
    return std::make_shared<AudioAsset>(j.at("name"), j.at("path"));
}