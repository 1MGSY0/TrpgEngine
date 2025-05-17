#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <json.hpp>
#include <imgui.h>
#include <glad/glad.h>

class AssetBase;

class Entity {
public:
    Entity() = default;
    Entity(const std::string& id, const std::string& name)
        : m_id(id), m_name(name) {}

    virtual ~Entity() = default;

    ImVec2 position;
    ImVec2 size;
    GLuint textureID;

    const std::string& getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void setId(const std::string& id) { m_id = id; }
    void setName(const std::string& name) { m_name = name; }

    virtual nlohmann::json toJson() const = 0;
    virtual void fromJson(const nlohmann::json& j);

    template<typename T>
    void addComponent(std::shared_ptr<T> comp) {
        m_components[typeid(T)] = comp;
    }

    template<typename T>
    std::shared_ptr<T> getComponent() {
        auto it = m_components.find(typeid(T));
        if (it != m_components.end())
            return std::static_pointer_cast<T>(it->second);
        return nullptr;
    }

protected:
    std::string m_id;
    std::string m_name;
    std::unordered_map<std::type_index, std::shared_ptr<AssetBase>> m_components;
};