#include "UI/MenuHelpers/EditorMenuHelpers.hpp"
#include "UI/EditorUI.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/ComponentRegistry.hpp"
#include "Engine/EntitySystem/Components/FlowNodeComponent.hpp"
#include "Engine/EntitySystem/Components/DialogueComponent.hpp"
#include "Engine/EntitySystem/Components/ChoiceComponent.hpp"
#include "Engine/EntitySystem/Components/DiceRollComponent.hpp"
#include "Engine/EntitySystem/Components/ProjectMetaComponent.hpp"
#include "Project/ProjectManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "Engine/RenderSystem/SceneManager.hpp"

using json = nlohmann::json;

namespace EditorMenuHelpers {

Entity getPreferredSceneNode() {
	auto& em = EntityManager::get();
	if (EditorUI* ui = EditorUI::get()) {
		Entity sel = ui->getSelectedEntity();
		if (em.hasComponent(sel, ComponentType::FlowNode))
			return sel;
		if (em.getComponent<DialogueComponent>(sel) ||
		    em.getComponent<ChoiceComponent>(sel) ||
		    em.getComponent<DiceRollComponent>(sel)) {
			return findOwnerScene(sel);
		}
	}
	return INVALID_ENTITY;
}

Entity createSceneFlowNode() {
	auto& em = EntityManager::get();

	Entity metaEntity = ProjectManager::getProjectMetaEntity();
	auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
	if (!base) return INVALID_ENTITY;
	auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

	Entity e = em.createEntity(INVALID_ENTITY);
	const auto* info = ComponentTypeRegistry::getInfo(ComponentType::FlowNode);
	if (!info || !info->loader) return INVALID_ENTITY;

	std::string defName = "Scene " + std::to_string(meta->sceneNodes.size() + 1);
	json init = json::object();
	init["name"] = defName;
	init["isStart"] = false;
	init["isEnd"] = false;
	init["nextNode"] = -1;
	init["characters"] = json::array();
	init["backgrounds"] = json::array();
	init["uiLayer"] = json::array();
	init["objectLayer"] = json::array();
	init["eventSequence"] = json::array();

	auto comp = info->loader(init);
	if (!comp || em.addComponent(e, comp) != EntityManager::AddComponentResult::Ok) return INVALID_ENTITY;
	comp->Init(e);

	if (auto fn = em.getComponent<FlowNodeComponent>(e)) {
		if (fn->name.empty()) fn->name = defName;
	}

	meta->sceneNodes.push_back(e);

	if (meta->startNode == INVALID_ENTITY) {
		meta->startNode = e;
		if (auto fn = em.getComponent<FlowNodeComponent>(e)) fn->isStart = true;
	}

	if (EditorUI* ui = EditorUI::get()) {
		Entity selected = ui->getSelectedEntity();
		if (auto selFn = em.getComponent<FlowNodeComponent>(selected)) {
			if (selFn->nextNode < 0) selFn->nextNode = static_cast<int>(e);
		}
		ui->setSelectedEntity(e);
	}

	SceneManager::get().setCurrentFlowNode(e);
	ResourceManager::get().setUnsavedChanges(true);
	return e;
}

Entity findOwnerScene(Entity evt) {
	auto& em = EntityManager::get();
	Entity metaEntity = ProjectManager::getProjectMetaEntity();
	auto base = em.getComponent(metaEntity, ComponentType::ProjectMetadata);
	if (!base) return INVALID_ENTITY;
	auto meta = std::static_pointer_cast<ProjectMetaComponent>(base);

	for (Entity n : meta->sceneNodes) {
		auto fn = em.getComponent<FlowNodeComponent>(n);
		if (!fn) continue;
		for (Entity e : fn->eventSequence) if (e == evt) return n;
	}
	return INVALID_ENTITY;
}

bool attachEventToScene(Entity evt, Entity sceneNode) {
	auto& em = EntityManager::get();
	if (evt == INVALID_ENTITY || sceneNode == INVALID_ENTITY) return false;

	// Remove from previous owner
	if (Entity owner = findOwnerScene(evt); owner != INVALID_ENTITY) {
		if (auto ownFn = em.getComponent<FlowNodeComponent>(owner)) {
			ownFn->eventSequence.erase(std::remove(ownFn->eventSequence.begin(), ownFn->eventSequence.end(), evt),
			                           ownFn->eventSequence.end());
		}
	}

	// Attach to destination
	if (auto dst = em.getComponent<FlowNodeComponent>(sceneNode)) {
		dst->eventSequence.push_back(evt);
		// Refresh SceneManager so Hierarchy and ScenePanel reflect immediately
		SceneManager::get().setCurrentFlowNode(sceneNode);
		ResourceManager::get().setUnsavedChanges(true);
		return true;
	}
	return false;
}

Entity createEventAndAttach(ComponentType type, Entity sceneNode) {
	auto& em = EntityManager::get();
	const auto* info = ComponentTypeRegistry::getInfo(type);
	if (!info || !info->loader) return INVALID_ENTITY;

	json init = json::object();
	if (type == ComponentType::Dialogue) {
		init["lines"] = json::array();
		init["speaker"] = -1;
		init["advanceOnClick"] = true;
		init["targetFlowNode"] = "";
	} else if (type == ComponentType::Choice) {
		init["options"] = json::array();
	} else if (type == ComponentType::DiceRoll) {
		init["sides"] = 6; init["threshold"] = 1; init["onSuccess"] = ""; init["onFailure"] = "";
	}

	Entity e = em.createEntity(INVALID_ENTITY);
	auto c = info->loader(init);
	if (!c || em.addComponent(e, c) != EntityManager::AddComponentResult::Ok) return INVALID_ENTITY;
	c->Init(e);

	if (!attachEventToScene(e, sceneNode)) return INVALID_ENTITY;

	if (EditorUI* ui = EditorUI::get()) ui->setSelectedEntity(e);
	ResourceManager::get().setUnsavedChanges(true);
	return e;
}

} // namespace EditorMenuHelpers