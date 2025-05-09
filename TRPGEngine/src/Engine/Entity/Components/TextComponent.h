#pragma once
#include "Engine/Entity/Entity.h"
#include <json.hpp>

class TextComponent : public Entity {
public:
    TextComponent() = default;

    TextComponent(const std::string& id, const std::string& name, const std::string& content)
        : Entity(id, name), m_content(content) {}

    // --- Accessors ---
    const std::string& getContent() const { return m_content; }
    void setContent(const std::string& content) { m_content = content; }

    const std::string& getLanguage() const { return m_language; }
    void setLanguage(const std::string& lang) { m_language = lang; }

    const std::string& getTag() const { return m_tag; }
    void setTag(const std::string& tag) { m_tag = tag; }

    int getFontSize() const { return m_fontSize; }
    void setFontSize(int size) { m_fontSize = size; }

    // --- Reference Accessors for ImGui ---
    std::string& getContentRef() { return m_content; }
    std::string& getLanguageRef() { return m_language; }
    std::string& getTagRef() { return m_tag; }
    int& getFontSizeRef() { return m_fontSize; }

    // --- Serialization ---
    nlohmann::json toJson() const override {
        return {
            {"id", m_id},
            {"name", m_name},
            {"content", m_content},
            {"language", m_language},
            {"tag", m_tag},
            {"fontSize", m_fontSize}
        };
    }

    void fromJson(const nlohmann::json& j) override {
        m_id = j.at("id").get<std::string>();
        m_name = j.at("name").get<std::string>();
        m_content = j.at("content").get<std::string>();
        m_language = j.value("language", "English");
        m_tag = j.value("tag", "default");
        m_fontSize = j.value("fontSize", 16);
    }

private:
    std::string m_content;
    std::string m_language = "English";
    std::string m_tag = "default";
    int m_fontSize = 16;
};
