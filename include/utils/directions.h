#pragma once
#include <unordered_map>

enum class Direction
{
    FORWARD,
    BACKWARDS,
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

const std::unordered_map<Direction, glm::vec3> directionVectors = {
    {Direction::FORWARD, glm::vec3{0, 0, -1}},
    {Direction::BACKWARDS, glm::vec3{0, 0, 1}},
    {Direction::LEFT, glm::vec3{-1, 0, 0}},
    {Direction::RIGHT, glm::vec3{1, 0, 0}},
    {Direction::UP, glm::vec3{0, 1, 0}},
    {Direction::DOWN, glm::vec3{0, -1, 0}}
};

inline const glm::vec3& getVector(const Direction d)
{
    return directionVectors.at(d);
}