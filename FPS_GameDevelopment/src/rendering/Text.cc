#include "Text.hh"

Text::Text(const std::string text, int x, int y, int size, Color fontColor)
{
	mTextString = text;
	mX = x;
	mY = y;
	mSize = size;
	mFontColor = fontColor;
}
