#include "Render3DSystem.hpp"
#include "Engine/EntitySystem/EntityManager.hpp"
#include "Engine/EntitySystem/Components/TransformComponent.hpp"
#include "Engine/EntitySystem/Components/ModelComponent.hpp"
#include "Engine/Graphics/ShaderUtils.hpp"
#include <gtc/type_ptr.hpp>

// Shader handle (should eventually be an asset)
GLuint g_shaderProgram = 0;

void Render3DSystem::init() {
    // Load and compile shaders (vertex + fragment)
    g_shaderProgram = createShaderProgram();  // Hardcoded inline function in ShaderUtils.hpp
}

void Render3DSystem::beginScene() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render3DSystem::renderEntityEditor(Entity e) {
    auto& em = EntityManager::get();
    auto obj = em.getComponent<ModelComponent>(e);
    auto tf  = em.getComponent<TransformComponent>(e);
    if (!obj || !tf || !obj->isLoaded || !obj->isVisible) return;

    glUseProgram(g_shaderProgram);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), tf->position);
    model = glm::rotate(model, glm::radians(tf->rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(tf->rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(tf->rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, tf->scale);

    // Example: set uniforms (assumes shader uses "u_Model")
    GLint modelLoc = glGetUniformLocation(g_shaderProgram, "u_Model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    for (auto& mesh : obj->meshes)
        mesh.draw();
}

void renderEntityRuntime(Entity e) {
    auto& em = EntityManager::get();
    //
}

void Render3DSystem::endScene() {
    glUseProgram(0);
}

void Render3DSystem::shutdown() {
    glDeleteProgram(g_shaderProgram);
}
