#pragma once
#include <renderobject.h>
#include <scene.h>
#include <gpuobjects/shader.h>

#include "renderer.h"

class TexturedModel : public RenderObject
{
public:
    TexturedModel(const Shader& shader, const Renderer& renderer,
                  const std::vector<std::reference_wrapper<const Texture>>& textures, const Model& model,
                  const SceneObject& scene_object): RenderObject(shader, renderer, textures, model, scene_object)
    {
    }

    void draw() const
    {
        draw(sceneObject.modelMatrix, renderer.viewMatrix(), renderer.projectionMatrix());
    }

    void draw(const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) const
    {
        bindTextures();
        shader.use();
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(projectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "modelMatrix"), 1, GL_FALSE,
                           value_ptr(modelMatrix));
        model.Draw();
    }
};
