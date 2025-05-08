#include "TextAsset.h"

TextAsset::TextAsset(const std::string& name, const std::string& content)
    : m_name(name), m_content(content) {}

std::string TextAsset::getName() const { return m_name; }

std::string TextAsset::getContent() const { return m_content; }

void TextAsset::setContent(const std::string& newContent) {
    m_content = newContent;
}

json TextAsset::toJson() const {
    return json{{"name", m_name}, {"content", m_content}};
}

std::shared_ptr<TextAsset> TextAsset::fromJson(const json& j) {
    std::string name = j.at("name").get<std::string>();
    std::string content = j.at("content").get<std::string>();
    return std::make_shared<TextAsset>(name, content);
}
