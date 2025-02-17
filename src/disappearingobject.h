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
        if (curThreshold >= 1)
        {
            return;
        }
        shader.use();
        bindTextures();

        glUniform1i(glGetUniformLocation(shader.program(), "texSampler"), 0);
        glUniform1i(glGetUniformLocation(shader.program(), "maskSampler"), 1);

        glUniform1f(glGetUniformLocation(shader.program(), "threshold"), curThreshold);
        glUniform1f(glGetUniformLocation(shader.program(), "lowerBoundThreshold"), 0);
        glUniform1i(glGetUniformLocation(shader.program(), "invert"), false);
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.viewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "modelMatrix"), 1, GL_FALSE,
                           value_ptr(sceneObject.worldModelMatrix()));
        shader.validateProgram();
        model.Draw();
    }

    void drawRemovedFragments() const
    {
        shader.use();
        bindTextures();

        glUniform1i(glGetUniformLocation(shader.program(), "texSampler"), 0);
        glUniform1i(glGetUniformLocation(shader.program(), "maskSampler"), 1);

        glUniform1f(glGetUniformLocation(shader.program(), "threshold"), curThreshold);
        glUniform1f(glGetUniformLocation(shader.program(), "lowerBoundThreshold"), prevThreshold);
        glUniform1i(glGetUniformLocation(shader.program(), "invert"), true);
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(renderer.viewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader.program(), "modelMatrix"), 1, GL_FALSE,
                           value_ptr(sceneObject.worldModelMatrix()));
        shader.validateProgram();
        model.Draw();
    }


    [[nodiscard]] float threshold() const
    {
        return curThreshold;
    }

    void threshold(const float threshold)
    {
        prevThreshold = curThreshold;
        curThreshold = glm::clamp(threshold, 0.0f, 1.0f);
    }

private:
    float curThreshold{0};
    float prevThreshold{0};
};
