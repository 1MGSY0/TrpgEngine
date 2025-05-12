#pragma once

#include <json.hpp>
#include "ComponentType.hpp"

class ComponentBase {
public:
    virtual ~ComponentBase() = default;

    virtual std::string getID() const = 0;
    virtual nlohmann::json toJson() const = 0;
    virtual ComponentType getType() const = 0;
};