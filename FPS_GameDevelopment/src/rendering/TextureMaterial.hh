#pragma once

#include <string>
#include <map>
#include <glm/vec2.hpp>
#include <glow/common/property.hh>
#include <glow/objects/Texture2D.hh>

enum TextureType
{
	TEX_DIFFUSE,
	TEX_NORMAL
};

class TextureMaterial
{
private: // members

	/// Texture type
	TextureType mTextureTypeString;

	/// Texture path
	std::string mPath;

	/// Color space
	std::string mColorSpace;

	/// Diffuse color
	glm::vec2 mScale = glm::vec2(1.0);

	/// Specular color
	glm::vec2 mOffset = glm::vec2(0.0);

	/// Texture 2D
	glow::SharedTexture2D mTextureValue;

public: // getter, setter
	GLOW_GETTER(TextureTypeString);

	GLOW_PROPERTY(Path);
	GLOW_PROPERTY(ColorSpace);
	GLOW_PROPERTY(Scale);
	GLOW_PROPERTY(Offset);
	GLOW_PROPERTY(TextureValue);

public: // functions
	TextureMaterial(TextureType const& textureType);
	TextureMaterial();
};
