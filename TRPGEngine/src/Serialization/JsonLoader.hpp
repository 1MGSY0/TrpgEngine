#pragma once
#include <string>
#include <memory>
#include <json.hpp>
#include "Engine/EntitySystem/EntityManager.hpp"

namespace JsonLoader {
    bool saveEntityToFile(Entity entity, const EntityManager& em, const std::string& path);
    Entity loadEntityFromFile(const std::string& path, EntityManager& em);
}
