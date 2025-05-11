// ComponentRegistry.h
#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <json.hpp>

#include "ComponentBase.h"
#include "ComponentType.h"

class ComponentRegistry {
public:
    static ComponentRegistry& get();
    using InspectorFunc = std::function<void(std::shared_ptr<ComponentBase>)>;

    static void add(const std::shared_ptr<ComponentBase>& component);
    static void clear();

    static void registerInspector(ComponentType type, InspectorFunc func);
    static void renderInspector(ComponentType type, const std::string& id);

    std::vector<std::shared_ptr<ComponentBase>> getComponents(ComponentType type) const;
    std::shared_ptr<ComponentBase> getComponentByID(ComponentType type, const std::string& id) const;
    std::vector<std::shared_ptr<ComponentBase>> getAllComponents() const;
    std::vector<std::shared_ptr<ComponentBase>>& getMutableComponents(ComponentType type);

private:
    static std::unordered_map<ComponentType, std::vector<std::shared_ptr<ComponentBase>>> m_components;
    static std::unordered_map<ComponentType, InspectorFunc> m_inspectors;
};