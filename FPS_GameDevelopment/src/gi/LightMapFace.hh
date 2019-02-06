#pragma once

#include <glm/glm.hpp>

class GameObject;

struct LightMapFace
{
    /// positions of the three triangle corners in world space
    glm::vec3 corners[3];

    /// uv coordinates of the corners in the light map
    glm::vec2 lightMapUVs[3];

    /// the GameObject this triangle belongs to
    GameObject* gameObject = nullptr;
};
