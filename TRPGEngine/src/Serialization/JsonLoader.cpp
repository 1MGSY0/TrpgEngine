#include "JsonLoader.hpp"
#include "Engine/EntitySystem/ComponentTypeRegistry.hpp"

using json = nlohmann::json;

namespace JsonLoader {

bool saveEntityToFile(Entity entity, const EntityManager& em, const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) return false;

    json jEntity;
    auto components = em.getAllComponents(entity);
    for (auto& comp : components) {
        jEntity[ComponentTypeRegistry::getInfo(comp->getType())->key] = comp->toJson();
    }

    out << jEntity.dump(4);
    return true;
}

Entity loadEntityFromFile(const std::string& path, EntityManager& em) {
    std::ifstream in(path);
    if (!in.is_open()) return INVALID_ENTITY;

    json j;
    in >> j;

    Entity entity = em.createEntity();

    for (const auto& info : ComponentTypeRegistry::getAllInfos()) {
        if (j.contains(info.key)) {
            auto comp = info.loader(j[info.key]);
            if (comp) em.addComponent(entity, comp);
        }
    }

    return entity;
}
}
