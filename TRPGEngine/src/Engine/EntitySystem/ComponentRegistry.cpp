// ComponentRegistry.cpp
#include "ComponentRegistry.hpp"
#include "ComponentType.hpp"
#include <imgui.h>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

std::unordered_map<ComponentType, std::vector<std::shared_ptr<ComponentBase>>> ComponentRegistry::m_components;
std::unordered_map<ComponentType, ComponentRegistry::InspectorFunc> ComponentRegistry::m_inspectors;

ComponentRegistry& ComponentRegistry::get() {
    static ComponentRegistry instance;
    return instance;
}

void ComponentRegistry::add(const std::shared_ptr<ComponentBase>& component) {
    m_components[component->getType()].push_back(component);
}

std::vector<std::shared_ptr<ComponentBase>> ComponentRegistry::getComponents(ComponentType type) const {
    auto it = m_components.find(type);
    if (it != m_components.end()) return it->second;
    return {};
}

std::shared_ptr<ComponentBase> ComponentRegistry::getComponentByID(ComponentType type, const std::string& id) const {
    auto it = m_components.find(type);
    if (it != m_components.end()) {
        for (const auto& component : it->second) {
            if (component->getID() == id) return component;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<ComponentBase>> ComponentRegistry::getAllComponents() const {
    std::vector<std::shared_ptr<ComponentBase>> all;
    for (const auto& [_, comps] : m_components) {
        all.insert(all.end(), comps.begin(), comps.end());
    }
    return all;
}

std::vector<std::shared_ptr<ComponentBase>>& ComponentRegistry::getMutableComponents(ComponentType type) {
    return m_components[type];
}


void ComponentRegistry::clear() {
    m_components.clear();
}

void ComponentRegistry::registerInspector(ComponentType type, InspectorFunc func) {
    m_inspectors[type] = std::move(func);
}

void ComponentRegistry::renderInspector(ComponentType type, const std::string& id) {
    auto component = ComponentRegistry::get().getComponentByID(type, id);
    if (!component) {
        ImGui::Text("Component not found.");
        return;
    }

    auto it = m_inspectors.find(type);
    if (it != m_inspectors.end()) {
        it->second(component);
    } else {
        ImGui::Text("No inspector registered for this component type.");
    }
}
