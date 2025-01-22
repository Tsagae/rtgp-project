#pragma once

#include <algorithm>
#include <vector>
#include <utils/directions.h>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/vector_angle.hpp>

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
        if (length2(groundDirection) < glm::epsilon<float>())
        {
            return;
        }

        groundDirection =  getOrientation() * groundDirection;
        groundDirection.y = verticalDirection.y;
        auto newPosition = getPosition() + normalize(groundDirection) * magnitude * dt;
        transform = glm::mat4{getOrientation()};
        transform[3] = glm::vec4{newPosition, 1};
    }

    void mouseRotate(float dx, float dy)
    {
        yaw += -dx * sensitivity;

        pitch += -dy * sensitivity;
        pitch = glm::clamp(pitch, -89.f, +89.f);

        transform = translate(glm::mat4{1}, getPosition()) *
            glm::yawPitchRoll(glm::radians(yaw), glm::radians(pitch), 0.f);
    }

    [[nodiscard]] glm::vec3 cameraForward() const
    {
        return normalize(cross(getVector(Direction::UP), getOrientation()[0]));
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
