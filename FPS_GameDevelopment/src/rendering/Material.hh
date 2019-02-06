#pragma once
#include <string>
#include <map>
#include <glm/vec3.hpp>
#include <glow/common/property.hh>
#include <rendering/TextureMaterial.hh>
#include <vector>
class Material
{
private: // members
		 /// Material name
	std::string mName;
	/// Opacity
	float mAlpha = 1.0;
	/// Scaling of level ambient color
	float mAmbient = 1.0;
	/// Diffuse color
	glm::vec3 mDiffuseColor = glm::vec3(0.6f);
	/// Specular color
	glm::vec3 mSpecularColor = glm::vec3(0.4f);
	/// Specular exponent
	float mSpecularHardness = 32.0;
	/// If true, material is modulated by object color
	bool mUseObjectColor = false;
	/// Texture Diffuse
	TextureMaterial mTextureDiffuse;
	/// Texture Normal
	TextureMaterial mTextureNormal;
	/// Custom property map
	std::map<std::string, float> mCustomProperties;
public: // getter, setter
	GLOW_GETTER(Name);
	GLOW_PROPERTY(Alpha);
	GLOW_PROPERTY(Ambient);
	GLOW_PROPERTY(DiffuseColor);
	GLOW_PROPERTY(SpecularColor);
	GLOW_PROPERTY(SpecularHardness);
	GLOW_PROPERTY(UseObjectColor);
	GLOW_PROPERTY(CustomProperties);
	GLOW_PROPERTY(TextureDiffuse);
	GLOW_PROPERTY(TextureNormal);
public: // functions
	Material(std::string const& name);
};