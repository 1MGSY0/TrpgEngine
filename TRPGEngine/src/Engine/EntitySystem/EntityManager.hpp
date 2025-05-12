#pragma once

#include "Entity.hpp"
#include "ComponentBase.hpp"
#include "ComponentType.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

class EntityManager {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);

    void addComponent(Entity entity, std::shared_ptr<ComponentBase> component);
    std::shared_ptr<ComponentBase> getComponent(Entity entity, ComponentType type);
    std::vector<std::shared_ptr<ComponentBase>> getAllComponents(Entity entity);
    bool hasComponent(Entity entity, ComponentType type);

private:
    Entity m_nextId = 1;
    std::unordered_map<Entity, std::unordered_map<ComponentType, std::shared_ptr<ComponentBase>>> m_entities;
};
