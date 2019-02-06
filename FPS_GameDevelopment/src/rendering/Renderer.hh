#pragma once

#include <vector>
#include <glm/vec2.hpp>

#include <glow/common/shared.hh>
#include <glow/common/property.hh>
#include "FontInterface.hh"
#include "MenuInterface.hh"
#include "Text.hh"

#include <glow/fwd.hh>

GLOW_SHARED(class, Renderer);
GLOW_SHARED(class, Level);

class Game;
/**
 * @brief The Renderer is responsible for drawing the current level (in OpenGL)
 */
class Renderer
{
private: // member
    /// Backreference to the game
    Game* const mGame;
	FontInterface fontInterface;
	MenuInterface menuInterface;

    /// Total seconds
    /// (careful, as a float, this has limited precision)
    float mTotalSeconds = 0.0f;

    /// If true, activates wireframe mode
    bool mWireframe = false;

	/// set text visibility
	bool mTextIsVisible = true;

private: // opengl
    /// common framebuffer object
    glow::SharedFramebuffer mFramebuffer;
    /// Framebuffer Texture: Color (RGB16F)
    glow::SharedTextureRectangle mTexBufferColor;
    /// Framebuffer Texture: Depth
    glow::SharedTextureRectangle mTexBufferDepth;   

	//textures
	glow::SharedTexture2D mTexColor;
	glow::SharedTexture2D mTexNormal;
	glow::SharedFramebuffer mFrameBufferTex;
	glm::vec2 mTexScale;

	//blur
	glow::SharedProgram mShaderBlur;
	glow::SharedTextureRectangle mTextureBlur;
	glow::SharedFramebuffer mFrameBufferBlur;
	//glow
	glow::SharedProgram mShaderGlow;
	glow::SharedTextureRectangle mTextureGlow;
	glow::SharedFramebuffer mFrameBufferGlow;

    /// Shader program responsible for drawing the framebuffer to the screen
    /// (also applies some simple post-process)
    glow::SharedProgram mShaderOutput;

    /// Shader program for drawing game objects
    glow::SharedProgram mShaderGameObjects;

    /// A simple quad
    glow::SharedVertexArray mMeshQuad;

    /// List of all fullscreen targets (for automatic resizing)
    std::vector<glow::SharedTextureRectangle> mTargets;

	///--------environment----
	glow::SharedProgram mShaderBG;
	glow::SharedVertexArray mQuadBG;
	glow::SharedTextureCubeMap mBackgroundTexture;

	///-------shadow-----
	glow::SharedProgram mShaderShadow;
	glow::SharedTextureRectangle mTextureShadow;
	glow::SharedFramebuffer mFrameBufferShadow;

	///----------------particle----------
	glow::SharedProgram mShaderParticle;
	
	glow::SharedArrayBuffer abParticleInstance;
	glow::SharedArrayBuffer abParticleBase;
	glow::SharedVertexArray mParticles;

	float randf(float min, float max);

	glow::SharedTexture2D mParticleTexture;
	
	//options 
	const int shadowSize = 512;

	// For font rendering
	std::map<std::string, Text> mTextsToRender;
	std::vector<std::string> mMenusToRender;
public: // getter, setter
    GLOW_GETTER(Game);

    GLOW_PROPERTY_IS(Wireframe);
	GLOW_PROPERTY(TextIsVisible);

public: // public functions
    /// ctor
    Renderer(Game* game);

    /// Render the game
    void render(float elapsedSeconds);

    ///-----scene render-------
	void renderScene();

	/// adding text to render at the end
	void addText(std::string key, std::string textString, int x, int y, int size, Color fontColor = WHITE);

	/// delete text based on key
	void deleteText(std::string key);

	/// add a new menu option
	void addMenu(std::string menuString);

	// swith menu on off
	void setMenuVisible(bool isVisible);

	// swith texts on off
	void setTextVisible(bool isVisible);

public: // events
    /// Called when the game is initialized
    /// CAUTION: No level loaded yet
    void onInit();

    /// Called after a level was loaded and initialized
    void onLevelLoaded(SharedLevel const& level);

    /// Called when the window is resized
    void onResize(int w, int h);
};
