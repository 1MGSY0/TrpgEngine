#include "ComponentType.hpp"
#include "ComponentBase.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"
#include "Engine/EntitySystem/Components/SceneMetaComponent.hpp"
#include "Engine/EntitySystem/Components/ParentComponent.hpp"
#include "Engine/EntitySystem/Components/ChildrenComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"


static std::vector<ComponentTypeInfo> componentTypeInfos;

namespace ComponentTypeRegistry {

    const ComponentTypeInfo* getInfo(ComponentType type) {
        for (const auto& info : componentTypeInfos)
            if (info.type == type)
                return &info;
        return nullptr;
    }

    const std::vector<ComponentTypeInfo>& getAllInfos() {
        return componentTypeInfos;
    }


    std::string ComponentTypeRegistry::getDefaultExtension(ComponentType type) {
        const auto* info = getInfo(type);
        if (!info || info->extensions.empty()) return ".json"; 
        return info->extensions.front(); 
    }


    void registerBuiltins() {
    componentTypeInfos.clear();

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::SceneMetadata,
        "scene",
        { ".json" },
        [](const nlohmann::json& j) { return SceneMetadataComponent::fromJson(j); },
        []() { return std::make_shared<SceneMetadataComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Parent,
        "parent",
        { ".json" },
        [](const nlohmann::json& j) { return ParentComponent::fromJson(j); },
        []() { return std::make_shared<ParentComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Children,
        "children",
        { ".json" },
        [](const nlohmann::json& j) { return ChildrenComponent::fromJson(j); },
        []() { return std::make_shared<ChildrenComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Character,
        "character",
        { ".json" },
        [](const nlohmann::json& j) { return CharacterComponent::fromJson(j); },
        []() { return std::make_shared<CharacterComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Script,
        "script",
        { ".lua", ".txt" },
        [](const nlohmann::json& j) { return ScriptComponent::fromJson(j); },
        []() { return std::make_shared<ScriptComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Dialogue,
        "dialogue",
        { ".json" },
        [](const nlohmann::json& j) { return DialogueComponent::fromJson(j); },
        []() { return std::make_shared<DialogueComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::FlowNode,
        "flownode",
        { ".json" },
        [](const nlohmann::json& j) { return FlowNodeComponent::fromJson(j); },
        []() { return std::make_shared<FlowNodeComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Transform,
        "transform",
        { ".json" },
        [](const nlohmann::json& j) { return TransformComponent::fromJson(j); },
        []() { return std::make_shared<TransformComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::Choice,
        "choice",
        { ".json" },
        [](const nlohmann::json& j) { return ChoiceComponent::fromJson(j); },
        []() { return std::make_shared<ChoiceComponent>(); }
    });

    componentTypeInfos.emplace_back(ComponentTypeInfo{
        ComponentType::DiceRoll,
        "dice",
        { ".json" },
        [](const nlohmann::json& j) { return DiceRollComponent::fromJson(j); },
        []() { return std::make_shared<DiceRollComponent>(); }
    });
}

    
}
