#pragma once

#include <vector>
#include <utils/directions.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>

class Camera
{
public:
    glm::mat4 transform{};
    float sensitivity{0.1};

    void addDirection(const Direction dir)
    {
        movementQueue.emplace_back(dir);
    }

    void move(const float magnitude, const float dt)
    {
        glm::vec3 movementVector{0};
        for (const auto& d : movementQueue)
        {
            movementVector += getVector(d);
        }
        movementQueue.clear();
        if (length2(movementVector) < glm::epsilon<float>())
        {
            return;
        }

        transform = translate(transform, normalize(movementVector) * magnitude * dt);
    }

    void mouseRotate(float dx, float dy)
    {
        yaw += -dx * sensitivity;
        yaw = glm::clamp(yaw, -89.f, +89.f);

        pitch += -dy * sensitivity;
        pitch = glm::clamp(pitch, -89.f, +89.f);

        transform = translate(glm::mat4{1}, getPosition()) *
            glm::yawPitchRoll(glm::radians(yaw), glm::radians(pitch), 0.f);
    }

    [[nodiscard]] glm::mat3 getOrientation() const
    {
        return transform;
    }

    [[nodiscard]] glm::vec3 getPosition() const
    {
        return transform[3];
    }

    [[nodiscard]] const glm::mat4& getTransform() const
    {
        return transform;
    }

private:
    std::vector<Direction> movementQueue{};
    float pitch{}, yaw{};
};
