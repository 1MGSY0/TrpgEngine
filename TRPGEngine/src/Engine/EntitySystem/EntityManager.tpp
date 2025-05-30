#pragma once

#include "EntityManager.hpp"

template<typename T>
std::shared_ptr<T> EntityManager::getComponent(Entity entity) {
    ComponentType type = T::getStaticType();
    auto it = m_entities.find(entity);
    if (it != m_entities.end()) {
        auto compIt = it->second.find(type);
        if (compIt != it->second.end()) {
            return std::dynamic_pointer_cast<T>(compIt->second);
        }
    }
    return nullptr;
}

template <typename T, typename... Args>
T& EntityManager::add(Entity e, Args&&... args) {
    auto comp = std::make_shared<T>(std::forward<Args>(args)...);
    addComponent(e, comp);
    return *comp;
}

template <typename T>
T* EntityManager::get(Entity e) {
    auto comp = getComponent<T>(e);
    return comp ? comp.get() : nullptr;
}
