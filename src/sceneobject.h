#pragma once

class SceneObject
{
public:
    glm::mat4 worldSpaceTransform = glm::mat4(1.f);
    glm::mat4 modelMatrix = glm::mat4(1.f);

    [[nodiscard]] glm::mat4 worldModelMatrix() const
    {
        return worldSpaceTransform * modelMatrix;
    }
};
