#pragma once

// Helper functions for bullet/glm interop

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <LinearMath/btVector3.h>
#include <LinearMath/btTransform.h>

inline glm::vec3 to_glm(btVector3 const& v)
{
    return {v.x(), v.y(), v.z()};
}

inline btVector3 to_bullet(glm::vec3 const& v)
{
    return {v.x, v.y, v.z};
}

inline glm::mat4 to_glm(btTransform const& t)
{
    glm::mat4 m;
    t.getOpenGLMatrix(&m[0][0]);
    return m;
}

inline btTransform to_bullet(glm::mat4 const& m)
{
    btTransform t;
    t.setFromOpenGLMatrix(&m[0][0]);
    return t;
}

