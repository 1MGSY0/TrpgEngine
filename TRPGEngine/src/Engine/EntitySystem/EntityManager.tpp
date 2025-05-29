// EntityManager.tpp
#include "EntityManager.hpp"
#include <memory>

// This tells the compiler you're defining a template function
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