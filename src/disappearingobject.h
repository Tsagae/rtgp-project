#pragma once
#include <renderobject.h>
#include <gpuobjects/shader.h>

#include "renderer.h"

class DisappearingObject : public RenderObject
{
public:
    DisappearingObject(const Shader& shader, const Renderer& renderer,
                       const std::vector<std::reference_wrapper<const Texture>>& textures, const Model& model,
                       const SceneObject& scene_object): RenderObject(shader, renderer, textures, model, scene_object)
    {
    }

    void draw() const
    {
        shader.use();
        bindTextures();

        glUniform1i(glGetUniformLocation(shader.program(), "texSampler"), 0);
        glUniform1i(glGetUniformLocation(shader.program(), "maskSampler"), 1);

        glUniform1f(glGetUniformLocation(shader.program(), "threshold"), _threshold);
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.viewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "modelMatrix"), 1, GL_FALSE,
                           value_ptr(sceneObject.worldModelMatrix()));
        model.Draw();
    }

    [[nodiscard]] float threshold() const
    {
        return _threshold;
    }

    void threshold(const float threshold)
    {
        _threshold = glm::clamp(threshold, 0.0f, 1.0f);
    }

private:
    float _threshold{0};
};
