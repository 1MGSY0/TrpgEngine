#pragma once
#include "Engine/Entity/Entity.h"
#include <string>
#include <unordered_map>
#include <json.hpp>

class CharacterComponent : public Entity {
public:
    CharacterComponent() = default;
    CharacterComponent(const std::string& id, const std::string& name)
        : Entity(id, name) {}

    // Sprite Path
    void setSpritePath(const std::string& path) { m_spritePath = path; }
    const std::string& getSpritePath() const { return m_spritePath; }
    std::string& getSpritePathRef() { return m_spritePath; }

    // Stats (key-value pairs like "HP", "Attack")
    void setStat(const std::string& key, int value) { m_stats[key] = value; }
    const std::unordered_map<std::string, int>& getStats() const { return m_stats; }
    std::unordered_map<std::string, int>& getStatsRef() { return m_stats; }

    // Serialization
    nlohmann::json toJson() const override {
        return {
            {"id", m_id},
            {"name", m_name},
            {"spritePath", m_spritePath},
            {"stats", m_stats}
        };
    }

    void fromJson(const nlohmann::json& j) override {
        m_id = j.at("id").get<std::string>();
        m_name = j.at("name").get<std::string>();
        m_spritePath = j.value("spritePath", "");
        m_stats = j.value("stats", std::unordered_map<std::string, int>{});
    }

private:
    std::string m_spritePath;
    std::unordered_map<std::string, int> m_stats;
};
