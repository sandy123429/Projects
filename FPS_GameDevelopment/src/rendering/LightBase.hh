#pragma once

#include <string>
#include <map>
#include <glm/vec3.hpp>
#include <glow/common/property.hh>
#include <glow/common/shared.hh>

GLOW_SHARED(class, LightBase);
/// Abstract base class for lights
class LightBase
{
private: // members
    /// Material name
    std::string mName;

    /// Light type name
    std::string mType;

    /// Light color
    glm::vec3 mColor = glm::vec3(1);

    /// Custom property map
    std::map<std::string, float> mCustomProperties;

public: // getter, setter
    GLOW_GETTER(Name);
    GLOW_PROPERTY(Type);

    GLOW_PROPERTY(Color);
    GLOW_PROPERTY(CustomProperties);

protected: // ctor
    LightBase(std::string const& name);
};
