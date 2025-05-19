#include "ComponentType.hpp"
#include "ComponentBase.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"
#include "Engine/EntitySystem/Components/SceneMetaComponent.hpp"
#include "Engine/EntitySystem/Components/ParentComponent.hpp"
#include "Engine/EntitySystem/Components/ChildrenComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"


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
            std::function<std::shared_ptr<ComponentBase>(const nlohmann::json&)>(
                [](const nlohmann::json& j) {
                    return CharacterComponent::fromJson(j);
                }),
            std::function<std::shared_ptr<ComponentBase>()>(
                []() {
                    return std::make_shared<CharacterComponent>();
                })
        });

        componentTypeInfos.emplace_back(ComponentTypeInfo{
            ComponentType::Script,
            "script",
            { ".lua", ".txt" },
            std::function<std::shared_ptr<ComponentBase>(const nlohmann::json&)>(
                [](const nlohmann::json& j) {
                    return ScriptComponent::fromJson(j);
                }),
            std::function<std::shared_ptr<ComponentBase>()>(
                []() {
                    return std::make_shared<ScriptComponent>();
                })
        });

        componentTypeInfos.emplace_back(ComponentTypeInfo{
            ComponentType::Dialogue,
            "dialogue",
            { ".json" },
            std::function<std::shared_ptr<ComponentBase>(const nlohmann::json&)>(
                [](const nlohmann::json& j) {
                    return DialogueComponent::fromJson(j);
                }),
            std::function<std::shared_ptr<ComponentBase>()>(
                []() {
                    return std::make_shared<DialogueComponent>();
                })
        });

        componentTypeInfos.emplace_back(ComponentTypeInfo{
            ComponentType::FlowNode,
            "flownode",
            { ".json" },
            std::function<std::shared_ptr<ComponentBase>(const nlohmann::json&)>(
                [](const nlohmann::json& j) {
                    return FlowNodeComponent::fromJson(j);
                }),
            std::function<std::shared_ptr<ComponentBase>()>(
                []() {
                    return std::make_shared<FlowNodeComponent>();
                })
        });

        // Add more component types here as needed
    }
    
}
