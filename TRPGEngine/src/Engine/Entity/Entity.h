#pragma once
#include <string>
#include <json.hpp>

class Entity {
public:
    Entity() = default;
    Entity(const std::string& id, const std::string& name)
        : m_id(id), m_name(name) {}

    virtual ~Entity() = default;

    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void setId(const std::string& id) { m_id = id; }
    void setName(const std::string& name) { m_name = name; }

    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& j) = 0;

protected:
    std::string m_id;
    std::string m_name;
};
