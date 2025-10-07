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

Entity EntityManager::createEntity(Entity parent) {
    Entity id = m_nextId++;
    m_metadata[id] = {};       // default meta
    m_entities[id] = {};       // component map for this entity

    if (parent != INVALID_ENTITY) {
        if (m_metadata.find(parent) == m_metadata.end()) {
            std::cerr << "[EntityManager] Parent " << parent << " does not exist.\n";
        } else {
            setEntityParent(id, parent);
        }
    }
    return id;
}

void EntityManager::destroyEntity(Entity entity) {
    m_entities.erase(entity);
}

bool EntityManager::entityExists(Entity e) const {
    return m_entities.find(e) != m_entities.end();
}

bool EntityManager::hasComponent(Entity e, ComponentType t) const {
    auto it = m_entities.find(e);
    if (it == m_entities.end()) return false;
    return it->second.find(t) != it->second.end();
}

EntityManager::AddComponentResult
EntityManager::addComponent(Entity e, std::shared_ptr<ComponentBase> c) {
    if (e == INVALID_ENTITY) {
        std::cerr << "[EntityManager] Refusing to add to INVALID_ENTITY.\n";
        return AddComponentResult::InvalidEntityId;
    }
    auto it = m_entities.find(e);
    if (it == m_entities.end()) {
        std::cerr << "[EntityManager] Entity " << e << " not found. Did you call createEntity()?\n";
        return AddComponentResult::EntityNotFound;
    }
    if (!c) {
        std::cerr << "[EntityManager] Null component.\n";
        return AddComponentResult::NullComponent;
    }

    const ComponentType t = c->getType();
    if (it->second.find(t) != it->second.end()) {
        std::cerr << "[EntityManager] Entity " << e
                  << " already has component type " << static_cast<int>(t) << ". Skipping.\n";
        return AddComponentResult::AlreadyExists;
    }

    it->second[t] = std::move(c);
    return AddComponentResult::Ok;
}

bool EntityManager::removeComponent(Entity e, ComponentType t) {
    auto it = m_entities.find(e);
    if (it == m_entities.end()) return false;
    return it->second.erase(t) > 0;
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


std::vector<Entity> EntityManager::getAllEntities() const {
    std::vector<Entity> entities;
    for (const auto& [id, _] : m_entities) {
        entities.push_back(id);
    }
    return entities;
}

std::vector<Entity> EntityManager::getEntitiesWith(ComponentType t) const {
    std::vector<Entity> out;
    out.reserve(m_entities.size());
    for (const auto& kv : m_entities) {
        const Entity e = kv.first;
        const auto& cmap = kv.second;
        if (cmap.find(t) != cmap.end()) out.push_back(e);
    }
    return out;
}

nlohmann::json EntityManager::serializeEntity(Entity e) const {
    nlohmann::json j;
    if (m_entities.find(e) == m_entities.end()) {
        std::cerr << "[EntityManager] serializeEntity: entity " << e << " missing from m_entities\n";
        return j; // or throw, or assert
    }

    // Save metadata
    const auto* meta = getMeta(e);
    if (meta) {
        j["_meta"] = {
            {"name", meta->name},
            {"type", static_cast<int>(meta->type)},
            {"parent", meta->parent},
        };
    }

    // Save components using ComponentTypeRegistry
    for (const auto& [type, comp] : m_entities.at(e)) {
        const auto* reg = ComponentTypeRegistry::getInfo(type);
        if (!reg) {
            std::cerr << "[Serialization] Unknown registered component type: " << static_cast<int>(type) << "\n";
            continue;
        }

        nlohmann::json compJson = comp->toJson();
        compJson["type"] = reg->key;  // Add type string for deserialization
        j["components"].push_back(compJson);
    }

    // Save children recursively
    if (meta && !meta->children.empty()) {
        for (Entity child : meta->children) {
            j["children"].push_back(serializeEntity(child));
        }
    }

    return j;
}



Entity EntityManager::deserializeEntity(const nlohmann::json& j) {
    Entity e = createEntity();

    // Load metadata
    if (j.contains("_meta")) {
        const auto& metaJ = j["_meta"];
        setEntityMeta(
            e,
            metaJ.value("name", "Unnamed"),
            static_cast<EntityType>(metaJ.value("type", 0))
        );

        Entity parent = metaJ.value("parent", INVALID_ENTITY);
        if (parent != INVALID_ENTITY) setEntityParent(e, parent);
    }

    // Load components
    if (j.contains("components")) {
        for (const auto& compJ : j["components"]) {
            if (!compJ.contains("type")) {
                std::cerr << "[Deserialization] Skipping component with no type field\n";
                continue;
            }

            std::string typeStr = compJ["type"].get<std::string>();

            ComponentType type;
            try {
                type = ComponentTypeRegistry::getTypeFromString(typeStr);
            } catch (const std::exception& ex) {
                std::cerr << "[Deserialization] " << ex.what() << "\n";
                continue;
            }

            const auto* reg = ComponentTypeRegistry::getInfo(type);
            if (!reg || !reg->loader) {
                std::cerr << "[Deserialization] No loader registered for type: " << typeStr << "\n";
                continue;
            }

            std::shared_ptr<ComponentBase> comp = reg->loader(compJ);
            addComponent(e, comp);  // assuming this adds the component to entity `e`
        }
    }

    // Load children recursively
    if (j.contains("children")) {
        for (const auto& childJ : j["children"]) {
            Entity child = deserializeEntity(childJ);
            setEntityParent(child, e);
        }
    }

    return e;
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

void EntityManager::setEntityMeta(Entity e, const std::string& name, EntityType type) {
    m_metadata[e] = EntityMeta{name, type};
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

void EntityManager::setSelectedEntity(Entity entity) {
    if (m_entities.find(entity) != m_entities.end()) {
        m_selectedEntity = entity;
        std::cout << "[EntityManager] Selected entity: " << entity << "\n";
    } else {
        std::cerr << "[EntityManager] Attempted to select invalid entity: " << entity << "\n";
    }
}

Entity EntityManager::getSelectedEntity() const {
    return m_selectedEntity;
}

bool EntityManager::hasSelectedEntity() const {
    return m_entities.find(m_selectedEntity) != m_entities.end();
}

const EntityMeta* EntityManager::getMeta(Entity e) const {
    auto it = m_metadata.find(e);
    return it != m_metadata.end() ? &it->second : nullptr;
}

EntityMeta* EntityManager::getMeta(Entity e) {
    auto it = m_metadata.find(e);
    return it != m_metadata.end() ? &it->second : nullptr;
}

std::vector<Entity> EntityManager::getChildren(Entity parent) const {
    auto it = m_metadata.find(parent);
    if (it != m_metadata.end()) {
        return it->second.children;
    } else {
        std::cerr << "[EntityManager] getChildren: No metadata for entity " << parent << "\n";
        return {};
    }
}

Entity EntityManager::getParent(Entity child) const {
    auto it = m_metadata.find(child);
    if (it != m_metadata.end()) {
        return it->second.parent;
    } else {
        std::cerr << "[EntityManager] getParent: No metadata found for entity " << child << "\n";
        return INVALID_ENTITY;
    }
}

Entity EntityManager::getRoot(Entity child) const {
    auto it = m_metadata.find(child);
    if (it == m_metadata.end()) {
        std::cerr << "[EntityManager] getRoot: No metadata found for entity " << child << "\n";
        return INVALID_ENTITY;
    }

    Entity current = child;
    while (true) {
        auto meta = getMeta(current);
        if (!meta || meta->parent == INVALID_ENTITY) {
            return current;
        }
        current = meta->parent;
    }
}

// Debug functions
void EntityManager::printHierarchy(Entity root, int depth = 0) const {
    const auto* meta = getMeta(root);
    if (!meta) return;

    std::cout << std::string(depth * 2, ' ') << "- " << meta->name << " (ID: " << root << ")\n";
    for (Entity child : meta->children) {
        printHierarchy(child, depth + 1);
    }
}

