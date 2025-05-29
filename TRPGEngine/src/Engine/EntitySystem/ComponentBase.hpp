#pragma once

#include "Entity.hpp"
#include "ComponentType.hpp"
#include <json.hpp>

class ComponentBase {
public:
    virtual ~ComponentBase() = default;

    virtual std::string getID() const = 0;
    virtual nlohmann::json toJson() const = 0;
    virtual ComponentType getType() const = 0;

    virtual void Init(Entity& entity) {}          
    virtual void Update(float deltaTime) {}  
};