#pragma once

#include <vector>
#include <gpuobjects/shader.h>
#include "renderer.h"

class RenderObject
{
public:
    explicit RenderObject(const Shader& shader, const Renderer& renderer,
                          const std::vector<std::reference_wrapper<const Texture>>& textures,
                          const Model& model, const SceneObject& sceneObject)
        : shader(shader), renderer(renderer), model{model}, textures(textures), sceneObject{sceneObject}
    {
    }

    void bindTextures() const
    {
        GLuint i = 0;
        for (auto texture : textures)
        {
            texture.get().bind(i);
            i++;
        }
    }

protected:
    const Shader& shader;
    const Renderer& renderer;
    const Model& model;
    const std::vector<std::reference_wrapper<const Texture>> textures;
    const SceneObject& sceneObject;
};
