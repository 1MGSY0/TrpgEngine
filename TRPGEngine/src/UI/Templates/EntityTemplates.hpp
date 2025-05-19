// EntityTemplates.hpp
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Engine/EntitySystem/ComponentBase.hpp"

struct EntityTemplate {
    std::string name;
    std::vector<std::shared_ptr<ComponentBase>> components;
};

const std::vector<EntityTemplate>& getEntityTemplates();
