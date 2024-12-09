#pragma once
#include "renderer.h"

class ApplyTextureShader
{
public:
    explicit ApplyTextureShader(const Shader& shader, const Renderer& renderer)
        : _shader(shader), _renderer(renderer)
    {
    }

    void use(const glm::mat4& modelMatrix) const
    {
        _shader.use();
        glUniformMatrix4fv(glGetUniformLocation(_shader.program(), "projectionMatrix"), 1, GL_FALSE,
                           value_ptr(_renderer.projectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(_shader.program(), "viewMatrix"), 1, GL_FALSE,
                           value_ptr(_renderer.viewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(_shader.program(), "modelMatrix"), 1, GL_FALSE,
                           value_ptr(modelMatrix));
    }

private:
    const Shader& _shader;
    const Renderer& _renderer;
};
