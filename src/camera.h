#pragma once

#include <vector>
#include <utils/directions.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

class Camera
{
public:
    float sensitivity{0.1};

    void addDirection(const Direction dir)
    {
        movementQueue.emplace_back(dir); //TODO: can be optimized with a bitmask
    }

    void move(const float magnitude, const float dt)
    {
        glm::vec3 groundDirection{0};
        glm::vec3 verticalDirection{0};
        for (const auto& d : movementQueue)
        {
            if (d == Direction::UP || d == Direction::DOWN)
                verticalDirection += getVector(d);
            else
                groundDirection += getVector(d);
        }
        movementQueue.clear();

        groundDirection = orientation() * groundDirection;
        groundDirection.y = verticalDirection.y;
        if (length2(groundDirection) < glm::epsilon<float>())
        {
            return;
        }
        const auto newPosition = position() + normalize(groundDirection) * magnitude * dt;
        transform = glm::mat4{orientation()};
        transform[3] = glm::vec4{newPosition, 1};
    }

    void mouseRotate(const float dx, const float dy)
    {
        yaw += -dx * sensitivity;

        pitch += -dy * sensitivity;
        pitch = glm::clamp(pitch, -89.f, +89.f);

        const auto position = transform[3];
        this->transform = glm::yawPitchRoll(glm::radians(yaw), glm::radians(pitch), 0.f);
        this->transform[3] = position;
    }

    [[nodiscard]] glm::vec3 cameraForward() const
    {
        return normalize(cross(getVector(Direction::UP), orientation()[0]));
    }

    [[nodiscard]] glm::mat3 orientation() const
    {
        return transform;
    }

    [[nodiscard]] glm::vec3 position() const
    {
        return transform[3];
    }

    [[nodiscard]] const glm::mat4& getTransform() const
    {
        return transform;
    }

    void setTransform(const glm::mat4& transform)
    {
        this->transform = transform;
    }


private:
    glm::mat4 transform{};
    std::vector<Direction> movementQueue{};
    float pitch{}, yaw{};
};
