// #include <cstdint>
// #include <memory>
// #include <utility>
// #include "ComponentBase.hpp"
// #include "EntityManager.hpp"

// class EntityManager;

// class EntityHandle {
// public:
//     EntityHandle() = default;
//     EntityHandle(Entity id, EntityManager* manager)
//         : id(id), manager(manager) {}

//     Entity getID() const { return id; }
    

//     template<typename T, typename... Args>
//     T& AddComponent(Args&&... args) {
//         auto component = std::make_shared<T>(std::forward<Args>(args)...);
//         manager->addComponent(id, component);
//         return *component;
//     }

//     template<typename T>
//     T* GetComponent() const {
//         auto component = manager->getComponent<T>(id);
//         return component ? component.get() : nullptr;
//     }

//     bool IsValid() const { return id != INVALID_ENTITY && manager != nullptr; }

// private:
//     Entity id = INVALID_ENTITY;
//     EntityManager* manager = nullptr;
// };