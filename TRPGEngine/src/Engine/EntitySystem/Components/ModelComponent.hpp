#pragma once

#include "Engine/EntitySystem/ComponentBase.hpp"
#include "Engine/EntitySystem/ComponentType.hpp"
#include <vector>

typedef unsigned int GLuint;

struct Mesh {
    GLuint VAO, VBO, EBO;
    unsigned int indexCount;

    void draw() const;
};

class ModelComponent : public ComponentBase {
public:
    std::vector<Mesh> meshes;
    bool isLoaded = false;
    bool isVisible = true;

    std::string getID() const override { return "model"; }
    ComponentType getType() const override { return ComponentType::Model; }
    static ComponentType getStaticType() { return ComponentType::Model; }

    nlohmann::json toJson() const override {
        return {
            {"isLoaded", isLoaded},
            {"isVisible", isVisible}
        };
    }

    static std::shared_ptr<ModelComponent> fromJson(const nlohmann::json& j) {
        auto model = std::make_shared<ModelComponent>();
        if (j.contains("meshes") && j["meshes"].is_array()) {
            for (const auto& meshJson : j["meshes"]) {
                Mesh mesh;
                mesh.VAO = meshJson.value("VAO", 0);
                mesh.VBO = meshJson.value("VBO", 0);
                mesh.EBO = meshJson.value("EBO", 0);
                mesh.indexCount = meshJson.value("indexCount", 0);
                model->meshes.push_back(mesh);
            }
        }
        model->isLoaded = j.value("isLoaded", false);
        model->isVisible = j.value("isVisible", true);
        // Load meshes if needed (not implemented here)
        return model;
    }
};
