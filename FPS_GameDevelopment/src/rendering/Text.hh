#pragma once

#include <string>
#include <map>
#include <glm/vec2.hpp>
#include <glow/common/property.hh>
#include <glow/objects/Texture2D.hh>

enum Color
{
	RED,
	BLUE,
	GREEN,
	WHITE,
	BLACK,
};

class Text
{
private: // members

	/// Text to render
	std::string mTextString;

	/// X position
	int mX;

	/// Y position
	int mY;

	/// Text size
	int mSize;

	Color mFontColor;

public: // getter, setter
	GLOW_GETTER(TextString);
	GLOW_GETTER(X);
	GLOW_GETTER(Y);
	GLOW_GETTER(Size);
	GLOW_GETTER(FontColor);

public: // functions
	Text(const std::string text, int x, int y, int size, Color fontColor = WHITE);
};