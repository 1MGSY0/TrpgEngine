#include "EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

EntityManager& EntityManager::get() {
    static EntityManager instance;
    return instance;
}

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

std::vector<Entity> EntityManager::getAllEntities() const {
    std::vector<Entity> entities;
    for (const auto& [id, _] : m_entities) {
        entities.push_back(id);
    }
    return entities;
}

nlohmann::json EntityManager::serializeEntity(Entity entity) const {
    nlohmann::json j;
    auto it = m_entities.find(entity);
    if (it == m_entities.end()) return j;

    for (const auto& [type, comp] : it->second) {
        const auto* info = ComponentTypeRegistry::getInfo(type);
        if (info) {
            j[info->key] = comp->toJson();
        }
    }

    return j;
}

Entity EntityManager::deserializeEntity(const nlohmann::json& j) {
    Entity entity = createEntity();

    for (const auto& info : ComponentTypeRegistry::getAllInfos()) {
        if (j.contains(info.key)) {
            auto comp = info.loader(j[info.key]);
            if (comp) {
                addComponent(entity, comp);
            }
        }
    }

    return entity;
}

void EntityManager::loadEntitiesFromFolder(const std::string& folderPath) {
    namespace fs = std::filesystem;
    fs::path path(folderPath);

    if (!fs::exists(path) || !fs::is_directory(path)) {
        std::cerr << "[EntityManager] Folder does not exist: " << folderPath << "\n";
        return;
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == ".entity") {
            std::ifstream in(entry.path());
            if (in.is_open()) {
                nlohmann::json j;
                in >> j;
                deserializeEntity(j);
                std::cout << "[EntityManager] Loaded entity: " << entry.path().filename() << "\n";
            } else {
                std::cerr << "[EntityManager] Failed to open file: " << entry.path() << "\n";
            }
        }
    }
}

void EntityManager::clear() {
    m_entities.clear();
    m_nextId = 1;
}
