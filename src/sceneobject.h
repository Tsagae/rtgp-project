#pragma once

class SceneObject
{
public:
    glm::mat4 modelMatrix;

    SceneObject(const Model& model, const Texture& texture)
        : modelMatrix(1.), _model(model),
          _texture(texture)
    {
    }

    SceneObject(const Model& model, const Texture& texture, const glm::mat4& modelMatrix)
        : modelMatrix(modelMatrix), _model(model),
          _texture(texture)
    {
    }

    void draw() const
    {
        _texture.use();
        _model.Draw();
    }

private:
    const Model& _model;
    const Texture& _texture;
};
