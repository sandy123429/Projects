#pragma once

#include <string>
#include <map>
#include <glm/vec2.hpp>
#include <glow/common/property.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/common/shared.hh>

#include <glow/fwd.hh>
#include "FontInterface.hh"

class MenuInterface
{
private: // members
private: // opengl
	//textures
	FontInterface* mFontInterface;
	std::vector<std::string> mMenus;
	bool mIsVisible;

	int const mStartX = 100;
	int const mStartY = 420;
	int const mMarginY = 50;
	int const mFontSize = 50;

public: // getter, setter
	GLOW_PROPERTY(Menus);
	GLOW_PROPERTY(IsVisible);

public: // functions
	MenuInterface();
	void Init(FontInterface* const fontInterface);
	void PrintTextMenu();
	void AddMenu(std::string menu);
};