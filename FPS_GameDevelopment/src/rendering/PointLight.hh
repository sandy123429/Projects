#pragma once

#include "LightBase.hh"

/// A light coming from a certain point in space
class PointLight : public LightBase
{
private: // members
    /// Light position
    glm::vec3 mPosition;

public: // getter, setter
    GLOW_PROPERTY(Position);

public:
    PointLight(std::string const& name);
};
