#include "ComponentType.hpp"
#include "ComponentBase.hpp"
#include "Engine/EntitySystem/EntityManager.tpp"

#include "UI/ComponentPanel/RenderScriptPanel.hpp"
#include "UI/ComponentPanel/RenderCharacterPanel.hpp"
#include "UI/ComponentPanel/RenderDialoguePanel.hpp"
#include "UI/ComponentPanel/RenderChoicePanel.hpp"
#include "UI/ComponentPanel/RenderDicePanel.hpp"
#include "UI/ComponentPanel/RenderBackgroundPanel.hpp"
#include "UI/ComponentPanel/RenderFlowNodePanel.hpp"
#include "UI/ComponentPanel/RenderUIButtonPanel.hpp"
#include "UI/ComponentPanel/RenderTransform2DPanel.hpp"
#include "UI/ComponentPanel/RenderTransform3DPanel.hpp"


// UI renderers
extern void renderCharacterInspector(const std::shared_ptr<CharacterComponent>&);
extern void renderScriptInspector(const std::shared_ptr<ScriptComponent>&);
extern void renderDialogueInspector(const std::shared_ptr<DialogueComponent>&);
extern void renderChoiceInspector(const std::shared_ptr<ChoiceComponent>&);
extern void renderDiceInspector(const std::shared_ptr<DiceRollComponent>&);
extern void renderBackgroundInspector(const std::shared_ptr<BackgroundComponent>&);
extern void renderFlowNodeInspector(const std::shared_ptr<FlowNodeComponent>&);
extern void renderUIButtonInspector(const std::shared_ptr<UIButtonComponent>&);
extern void renderTransform3DInspector(const std::shared_ptr<TransformComponent>&);
extern void renderTransform2DInspector(const std::shared_ptr<Transform2DComponent>&);


namespace ComponentTypeRegistry {

static std::unordered_map<ComponentType, RegisteredComponent> componentsByType;
static std::unordered_map<std::string, ComponentType> stringToType;

void registerBuiltins() {
    componentsByType.clear();
    stringToType.clear();

    auto registerComponent = [](ComponentType type, const std::string& key, LoaderFn loader, InspectorRendererFn renderer = nullptr) {
        componentsByType[type] = RegisteredComponent{ loader, key, renderer };
        stringToType[key] = type;
    };

        registerComponent(ComponentType::ProjectMetadata, "project",
            [](const nlohmann::json& j) { return ProjectMetaComponent::fromJson(j); }
            // No inspector function provided
        );

    registerComponent(ComponentType::Character, "character",
        [](const nlohmann::json& j) { return CharacterComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderCharacterInspector(std::static_pointer_cast<CharacterComponent>(base));
        });

    registerComponent(ComponentType::Script, "script",
        [](const nlohmann::json& j) { return ScriptComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderScriptInspector(std::static_pointer_cast<ScriptComponent>(base));
        });

    registerComponent(ComponentType::Dialogue, "dialogue",
        [](const nlohmann::json& j) { return DialogueComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderDialogueInspector(std::static_pointer_cast<DialogueComponent>(base));
        });

    registerComponent(ComponentType::FlowNode, "flownode",
        [](const nlohmann::json& j) { return FlowNodeComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderFlowNodeInspector(std::static_pointer_cast<FlowNodeComponent>(base));
        });

    registerComponent(ComponentType::Transform, "transform",
        [](const nlohmann::json& j) { return TransformComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderTransform3DInspector(std::static_pointer_cast<TransformComponent>(base));
        });

    registerComponent(ComponentType::Transform2D, "transform2d",
        [](const nlohmann::json& j) { return Transform2DComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderTransform2DInspector(std::static_pointer_cast<Transform2DComponent>(base));
        });

    registerComponent(ComponentType::Choice, "choice",
        [](const nlohmann::json& j) { return ChoiceComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderChoiceInspector(std::static_pointer_cast<ChoiceComponent>(base));
        });

    registerComponent(ComponentType::DiceRoll, "dice",
        [](const nlohmann::json& j) { return DiceRollComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderDiceInspector(std::static_pointer_cast<DiceRollComponent>(base));
        });

    registerComponent(ComponentType::Background, "background",
        [](const nlohmann::json& j) { return BackgroundComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderBackgroundInspector(std::static_pointer_cast<BackgroundComponent>(base));
        });
    registerComponent(
        ComponentType::UIButton, "ui_button",
        [](const nlohmann::json& j) { return UIButtonComponent::fromJson(j); },
        [](std::shared_ptr<ComponentBase> base) {
            renderUIButtonInspector(std::static_pointer_cast<UIButtonComponent>(base));
        }
    );
}

const RegisteredComponent* getInfo(ComponentType type) {
    auto it = componentsByType.find(type);
    return it != componentsByType.end() ? &it->second : nullptr;
}

const std::unordered_map<ComponentType, RegisteredComponent>& getAllInfos() {
    return componentsByType;
}

}
