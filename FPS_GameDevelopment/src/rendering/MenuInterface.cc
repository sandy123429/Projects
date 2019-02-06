#include "MenuInterface.hh"

MenuInterface::MenuInterface()
{
}

void MenuInterface::Init(FontInterface * const fontInterface)
{
	// init font interface
	mFontInterface = fontInterface;
}

void MenuInterface::PrintTextMenu()
{
	if (!mIsVisible) {
		return;
	}

	int yNow = mStartY;

	// render menu for each menu text
	for (std::string const menuString : mMenus) {
		mFontInterface->PrintText2d(menuString, mStartX, yNow, 30, WHITE);
		yNow -= mMarginY;
	}
}

void MenuInterface::AddMenu(std::string menu)
{
	mMenus.push_back(menu);
}