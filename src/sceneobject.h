#pragma once

class SceneObject
{
public:
    glm::mat4 worldSpaceTransform = glm::mat4(1.f);
    glm::mat4 modelMatrix;

    explicit SceneObject(const glm::mat4& modelMatrix): modelMatrix(modelMatrix)
    {
    }

    [[nodiscard]] glm::mat4 worldModelMatrix() const
    {
        return worldSpaceTransform * modelMatrix;
    }
};
