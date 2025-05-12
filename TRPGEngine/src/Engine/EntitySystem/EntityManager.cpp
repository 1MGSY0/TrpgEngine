#include "EntityManager.hpp"

Entity EntityManager::createEntity() {
    return m_nextId++;
}

void EntityManager::destroyEntity(Entity entity) {
    m_entities.erase(entity);
}

void EntityManager::addComponent(Entity entity, std::shared_ptr<ComponentBase> component) {
    m_entities[entity][component->getType()] = component;
}

std::shared_ptr<ComponentBase> EntityManager::getComponent(Entity entity, ComponentType type) {
    auto it = m_entities.find(entity);
    if (it != m_entities.end()) {
        auto compIt = it->second.find(type);
        if (compIt != it->second.end()) return compIt->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<ComponentBase>> EntityManager::getAllComponents(Entity entity) {
    std::vector<std::shared_ptr<ComponentBase>> result;
    auto it = m_entities.find(entity);
    if (it != m_entities.end()) {
        for (const auto& [type, comp] : it->second) {
            result.push_back(comp);
        }
    }
    return result;
}

bool EntityManager::hasComponent(Entity entity, ComponentType type) {
    return getComponent(entity, type) != nullptr;
}
