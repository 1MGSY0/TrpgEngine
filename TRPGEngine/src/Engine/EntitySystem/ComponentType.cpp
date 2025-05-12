#include "ComponentType.hpp"
#include "ComponentBase.hpp"
#include "Engine/EntitySystem/Components/CharacterComponent.hpp"
#include "Engine/EntitySystem/Components/ScriptComponent.hpp"

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

    std::shared_ptr<ComponentBase> ComponentTypeRegistry::deserializeComponent(ComponentType type, const nlohmann::json& j) {
        const auto* info = getInfo(type);
        if (!info || !info->loader) return nullptr;
        return info->loader(j);
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
    }
    
}
