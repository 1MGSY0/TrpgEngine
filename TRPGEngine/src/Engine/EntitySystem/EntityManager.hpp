#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <json.hpp>
#include "Entity.hpp"
#include "ComponentBase.hpp"
#include "ComponentType.hpp"

struct EntityMeta {
    std::string name;
    EntityType type = EntityType::Default;
    Entity parent = INVALID_ENTITY;
    std::vector<Entity> children;
};

class EntityManager {
public:
    static EntityManager& get();

    Entity createEntity();
    void destroyEntity(Entity entity);
    void clear();

    
    void addComponent(Entity e, std::shared_ptr<ComponentBase> component);
    std::shared_ptr<ComponentBase> getComponent(Entity e, ComponentType type);
    bool hasComponent(Entity e, ComponentType type);
    std::vector<std::shared_ptr<ComponentBase>> getAllComponents(Entity e);

    std::vector<Entity> getAllEntities() const;

    // Metadata
    void setEntityName(Entity e, const std::string& name);
    void setEntityType(Entity e, EntityType type);
    void setEntityParent(Entity child, Entity parent);
    const EntityMeta* getMeta(Entity e) const;
    EntityMeta* getMeta(Entity e);

    // Hierarchy
    std::vector<Entity> getChildren(Entity parent) const;

    // IO
    nlohmann::json serializeEntity(Entity e) const;
    Entity deserializeEntity(const nlohmann::json& j);
    void loadEntitiesFromFolder(const std::string& path);

    // Handle-like API
    template <typename T, typename... Args>
    T& add(Entity e, Args&&... args);

    template <typename T>
    T* get(Entity e);

    template<typename T>
    std::shared_ptr<T> getComponent(Entity entity);

private:
    EntityManager() = default;

    Entity m_nextId = 1;
    std::unordered_map<Entity, std::unordered_map<ComponentType, std::shared_ptr<ComponentBase>>> m_entities;
    std::unordered_map<Entity, EntityMeta> m_metadata;

    // Disable copying
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
};

#include "EntityManager.tpp"

// #pragma once

// #include "Entity.hpp"
// #include "EntityMeta.hpp"
// #include "ComponentType.hpp"
// #include "ComponentBase.hpp"
// #include "ComponentRegistry.hpp"

// #include <unordered_map>
// #include <map>
// #include <string>
// #include <vector>
// #include <memory>
// #include <functional>

// class EntityManager {
// public:
//     static EntityManager& get();

//     Entity createEntity();
//     void destroyEntity(Entity entity);

//     void addComponent(Entity entity, std::shared_ptr<ComponentBase> component);
//     std::shared_ptr<ComponentBase> getComponent(Entity entity, ComponentType type);
//     std::vector<std::shared_ptr<ComponentBase>> getAllComponents(Entity entity);
//     bool hasComponent(Entity entity, ComponentType type);

//     std::vector<Entity> getAllEntities() const;
//     nlohmann::json serializeEntity(Entity entity) const;
//     Entity deserializeEntity(const nlohmann::json& j);
//     void loadEntitiesFromFolder(const std::string& folderPath);

//     void clear();

//     // Templated getter
//     template<typename T>
//     std::shared_ptr<T> getComponent(Entity entity);

//     // Metadata
//     void setEntityName(Entity e, const std::string& name);
//     void setEntityType(Entity e, EntityType type);
//     void setEntityParent(Entity child, Entity parent);
//     const EntityMeta* getMeta(Entity e) const;
//     EntityMeta* getMeta(Entity e);

// private:
//     EntityManager() = default;
//     Entity m_nextId = 1;
//     std::unordered_map<Entity, EntityMeta> m_metadata;
//     std::unordered_map<Entity, std::unordered_map<ComponentType, std::shared_ptr<ComponentBase>>> m_entities;
// };

// // Include template implementation
// #include "EntityManager.tpp"