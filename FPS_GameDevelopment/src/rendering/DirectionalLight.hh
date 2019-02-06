#pragma once

#include "LightBase.hh"

/// A light coming from infinity, shining to a certain direction
class DirectionalLight : public LightBase
{
private: // members
    /// Light direction (should be normalized)
    glm::vec3 mDirection;

public: // getter, setter
    GLOW_PROPERTY(Direction);

public:
    DirectionalLight(std::string const& name);
};
