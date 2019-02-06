#pragma once

#include <string>
#include <vector>
#include <glow/common/property.hh>

#include <AntTweakBar.h>

#include <glow-extras/camera/GenericCamera.hh>
#include <btBulletDynamicsCommon.h>

struct GLFWwindow;

GLOW_SHARED(class, Renderer);
GLOW_SHARED(class, Level);
/**
 * @brief The main Game class
 *
 * This class is the top-level manager of the game
 * It includes (and/or delegates):
 *   - rendering
 *   - physics
 *   - logic
 *   - level loading
 */
class Game
{
private:
    /// singleton instance
    static Game* sInstance;

private: // class member
    /// Name of the game (also used for window title)
    std::string mName = "GameDev Practical Template";

    /// RenderWindow
    GLFWwindow* mWindow = nullptr;

    /// Global game tweakbar
    //TwBar* mTweakBar = nullptr;

    /// Current game camera
    glow::camera::SharedGenericCamera mCamera;

    /// Object for rendering the game
    SharedRenderer mRenderer;
    /// Currently loaded level
    SharedLevel mLevel;

private: //parameters

	/// Game window width
	int mWindowWidth = -1;
	/// Game window height
	int mWindowHeight = -1;

    // input
    bool mKeyW = false;
    bool mKeyA = false;
    bool mKeyS = false;
    bool mKeyD = false;
	bool mKeySpace = false;
    bool mMouseRight = false;
	bool mMouseLeft = false;
    double mMouseLastX = -1;
    double mMouseLastY = -1;

	btVector3 mForwardLast = btVector3(0, 0, -1);
	float yaw = 0.3f;
	float sum_angle = 0;
	bool back = false;
	bool popmenu = false;

private: // private functions
		 /// no copy
	Game(Game const&) = delete;
	/// no copy assignment
	Game& operator=(Game const&) = delete;

public: // getter, setter

    GLOW_GETTER(Name);
    GLOW_GETTER(Window);
    GLOW_GETTER(WindowWidth);
    GLOW_GETTER(WindowHeight);
    //GLOW_GETTER(TweakBar);
    GLOW_GETTER(Renderer);
    GLOW_GETTER(Level);
    GLOW_PROPERTY(Camera);

public: //parameters
	bool isStart = false;
	bool timeout = false;
public: // public functions
    /// Get the Game instance everywhere by calling Game::the()
    static Game* the() { return sInstance; }
    /// ctor
    Game();

    /// Loads (and starts) a level by specifying a .blend file
    void loadLevel(std::string const& blendFileName);

public: // events

	void startGame(GLFWwindow *window, int argc, char **argv);
	void endGame();

    /// Called in the beginning
    /// Initialize and load your resources here
    /// Also processed command line arguments
    void onInit(GLFWwindow* window, int argc, char **argv);
	
    /// Called once per frame to update and render the game
    void onTick(float elapsedSeconds);

    /// Called once after init and then everytime the window is resized
    void onResize(int w, int h);

    /// Called whenever a key is pressed or released
    /// see http://www.glfw.org/docs/latest/group__input.html#ga0192a232a41e4e82948217c8ba94fdfd
    void onKeyPress(int key, int scancode, int action, int modifiers);

    /// Called whenever a unicode character input is generated
    /// (this function is intended for text input)
    /// see http://www.glfw.org/docs/latest/group__input.html#gabf24451c7ceb1952bc02b17a0d5c3e5f
    void onCharInput(uint32_t codepoint);

    /// Called whenever a mouse button is pressed
    /// see http://www.glfw.org/docs/latest/group__input.html#ga39893a4a7e7c3239c98d29c9e084350c
    void onMousePress(int button, int action, int modifiers);

    /// Called whenever a mouse move was detected
    /// (coordinates are relative to top-left of window area)
    /// see http://www.glfw.org/docs/latest/group__input.html#ga4cfad918fa836f09541e7b9acd36686c
    void onMouseMove(double x, double y);

    /// Called whenever the mouse wheel was used
    /// (e.g. yoffset is -1 if the wheel is scrolled one step down)
    /// http://www.glfw.org/docs/latest/group__input.html#ga4687e2199c60a18a8dd1da532e6d75c9
    void onMouseWheel(double xoffset, double yoffset);
};
