#include "EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

EntityManager& EntityManager::get() {
    static EntityManager instance;
    return instance;
}

void EntityManager::clear() {
    m_entities.clear();
    m_metadata.clear();
    m_nextId = 1;
}

Entity EntityManager::createEntity() {
    Entity id = m_nextId++;
    m_metadata[id] = EntityMeta();  // Initialize metadata
    return id;
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

nlohmann::json EntityManager::serializeEntity(Entity e) const {
    nlohmann::json j;
    auto it = m_entities.find(e);
    if (it == m_entities.end()) return j;

    for (const auto& [type, comp] : it->second) {
        const auto* info = ComponentTypeRegistry::getInfo(type);
        if (info) {
            j[info->key] = comp->toJson();
        }
    }

    const auto& meta = m_metadata.at(e);
    j["_meta"] = {
        {"name", meta.name},
        {"type", static_cast<int>(meta.type)},
        {"parent", meta.parent}
    };

    return j;
}

Entity EntityManager::deserializeEntity(const nlohmann::json& j) {
    Entity entity = createEntity();

    for (const auto& [type, info] : ComponentTypeRegistry::getAllInfos()) {
        if (j.contains(info.key)) {
            auto comp = info.loader(j[info.key]);
            if (comp) addComponent(entity, comp);
        }
    }

    if (j.contains("_meta")) {
        const auto& jm = j["_meta"];
        m_metadata[entity].name = jm.value("name", "Unnamed");
        m_metadata[entity].type = static_cast<EntityType>(jm.value("type", 0));
        m_metadata[entity].parent = jm.value("parent", INVALID_ENTITY);
        if (m_metadata[entity].parent != INVALID_ENTITY) {
            m_metadata[m_metadata[entity].parent].children.push_back(entity);
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

void EntityManager::setEntityName(Entity e, const std::string& name) {
    m_metadata[e].name = name;
}

void EntityManager::setEntityType(Entity e, EntityType type) {
    m_metadata[e].type = type;
}

void EntityManager::setEntityParent(Entity child, Entity parent) {
    m_metadata[child].parent = parent;
    m_metadata[parent].children.push_back(child);
}

const EntityMeta* EntityManager::getMeta(Entity e) const {
    auto it = m_metadata.find(e);
    return it != m_metadata.end() ? &it->second : nullptr;
}

EntityMeta* EntityManager::getMeta(Entity e) {
    auto it = m_metadata.find(e);
    return it != m_metadata.end() ? &it->second : nullptr;
}

