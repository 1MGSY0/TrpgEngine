#pragma once

#include <string>
#include <vector>
#include <functional>
#include <json.hpp>
#include <memory>

#include "ComponentBase.h"

enum class ComponentType {
    Unknown = 0,
    Character,
    Script,
    // Add more as needed
};

struct ComponentTypeInfo {
    ComponentType type;
    std::string key;
    std::vector<std::string> extensions;
    std::function<std::shared_ptr<ComponentBase>(const nlohmann::json&)> loader;
    std::function<std::shared_ptr<ComponentBase>()> factory; 
};

namespace ComponentTypeRegistry {
    const ComponentTypeInfo* getInfo(ComponentType type);
    const std::vector<ComponentTypeInfo>& getAllInfos();
    std::string getDefaultExtension(ComponentType type);
    std::shared_ptr<ComponentBase> deserializeComponent(ComponentType type, const nlohmann::json& j);
    void registerBuiltins();
}
