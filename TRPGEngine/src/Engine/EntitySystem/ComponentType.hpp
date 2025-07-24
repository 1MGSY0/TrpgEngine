#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <json.hpp>

class ComponentBase;

enum class ComponentType {
    Unknown = 0,
    ProjectMetadata,
    Character,
    Script,
    Choice,
    Dialogue,
    FlowNode,
    Model,
    Transform,
    Transform2D,
    UIButton,
    DiceRoll,
    Background,
};


namespace ComponentTypeRegistry {

ComponentType getTypeFromString(const std::string& key);

using LoaderFn = std::function<std::shared_ptr<ComponentBase>(const nlohmann::json&)>;
using ExtensionList = std::vector<std::string>;
using InspectorRendererFn = std::function<void(std::shared_ptr<ComponentBase>)>;

struct RegisteredComponent {
    LoaderFn loader;
    std::string key;
    InspectorRendererFn inspectorRenderer = nullptr;
};

void registerBuiltins();
const RegisteredComponent* getInfo(ComponentType type);
const std::unordered_map<ComponentType, RegisteredComponent>& getAllInfos();

}
