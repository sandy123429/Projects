#pragma once

#include <glow/common/property.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/common/shared.hh>

#include <glow/fwd.hh>
#include "Text.hh";

class FontInterface
{
private: // members
private: // opengl
	//textures
	glow::SharedTexture2D mFontTexture;
	glow::SharedProgram mShaderFont;
	glow::SharedProgram mShaderBackground;

public: // getter, setter
public: // functions
	FontInterface();
	void InitText2d(const std::string &texturePath);
	void PrintText2d(std::string text, int x, int y, int size, Color fontColor = WHITE);
	void DrawOverlay();
};