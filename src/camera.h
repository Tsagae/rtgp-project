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

        groundDirection = orientation() * groundDirection;
        groundDirection.y = verticalDirection.y;
        const auto newPosition = position() + normalize(groundDirection) * magnitude * dt;
        transform = glm::mat4{orientation()};
        transform[3] = glm::vec4{newPosition, 1};
    }

    void mouseRotate(const float dx, const float dy)
    {
        yaw += -dx * sensitivity;

        pitch += -dy * sensitivity;
        pitch = glm::clamp(pitch, -89.f, +89.f);

        transform = translate(glm::mat4{1}, position()) *
            glm::yawPitchRoll(glm::radians(yaw), glm::radians(pitch), 0.f);
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

    void setTransform(const glm::mat4& mat)
    {
        //TODO: computation of pitch and yaw is incorrect and redundant with the transform. Keep only pitch, yaw and position and compute transform on the fly
        transform = mat;
        const auto cam_orientation = orientation();
        yaw = glm::degrees(angle(getVector(Direction::FORWARD), -cam_orientation[2]));
        pitch = glm::degrees(angle(getVector(Direction::UP),  cam_orientation[1]));
    };

private:
    glm::mat4 transform{};
    std::vector<Direction> movementQueue{};
    float pitch{}, yaw{};
};
