#include "Game.hh"

#include <fstream>
#include<chrono>
#include<string>
// glow BEFORE glfw (due to ogl conflicts)
#include <glow/gl.hh>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include<math.h>
#include <common/assert.hh>
#include <common/atb_translate.hh>
#include <common/bullet_helper.hh>
#include <glow/common/log.hh>
#include <glow/common/scoped_gl.hh>

#include <aion/Action.hh>

#include <rendering/Renderer.hh>
#include <logic/Level.hh>
#include <logic/GameObject.hh>
#include <typeinfo>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btVector3.h>
#include <LinearMath/btScalar.h>

#include <rendering/Text.hh>



using namespace glow;
using namespace glow::camera;
//using namespace std::string_literals;
Game* Game::sInstance = nullptr;

Game::Game()
{
    gdassert(sInstance == nullptr, "Game can only be created once");
    sInstance = this;

    // do not load resources here but rather in Game::init()
}

void Game::startGame(GLFWwindow *window, int argc, char **argv) {
	onInit(window, argc, argv);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	onResize(width, height);	
}

void Game::endGame() {

}
btClock clkFruit;
void Game::loadLevel(const std::string& blendFileName)
{
    ACTION(); // measure time used by this function

    // create new level
    mLevel = std::make_shared<Level>(this);

    // load level
    mLevel->loadFromFile(blendFileName);

    //mLevel->loadFromFile("level/objects/Apple.blend");
    //mLevel->loadFromFile("level/objects/Banana.blend");
    //mLevel->loadFromFile("level/objects/Bomb.blend");
    //mLevel->loadFromFile("level/objects/Lemon.blend");
    //mLevel->loadFromFile("level/objects/Orange.blend");
    //mLevel->loadFromFile("level/objects/Pumpkin.blend");
    //mLevel->loadFromFile("level/objects/straw_try.blend");*/
	//setup character controller
	mLevel->setupCharacterController();

    // init level
    mLevel->onInit();

    // notify renderer
    mRenderer->onLevelLoaded(mLevel);
	
	if (mLevel->findGameObject("Katana")) {
		auto obj = mLevel->findGameObject("Katana");
		btTransform t;
		obj->getMotionState()->getWorldTransform(t);
		btVector3 fD = mLevel->getGhostObject()->getWorldTransform().getBasis()[2];
		t.setOrigin(mLevel->getGhostObject()->getWorldTransform().getOrigin() + fD * 3);
		obj->getMotionState()->setWorldTransform(t);
	}
}

void Game::onInit(GLFWwindow* window, int argc, char** argv)
{
    ACTION(); // measure time used by this function

    // assign window
    gdassert(mWindow == nullptr, "Init was already called");
    mWindow = window;

    // assign window size
    glfwGetWindowSize(window, &mWindowWidth, &mWindowHeight);

    // create default camera
    mCamera = std::make_shared<GenericCamera>();
	mCamera->setPosition(to_glm(btVector3(0,1,0)));
	mCamera->setTarget(to_glm(btVector3(0, 1, 4)));
    // create default renderer
    mRenderer = std::make_shared<Renderer>(this);
    mRenderer->onInit();

    // check if run in correct path
    if (!std::ifstream("level/exporter.py").good())
    {
        std::cout << "Game must be run from 'bin/' folder" << std::endl;
        std::cout << "(check the working directory)" << std::endl;
        exit(EXIT_FAILURE);
    }

    // parse args
    if (argc == 2) // one arg = level
    {
        loadLevel(argv[1]);
    }
    else
    {
        std::cout << "Usage: ~ LevelFile.blend" << std::endl;
        exit(EXIT_SUCCESS);
    }

	// TODO : Remove, for testing only
	
	// mRenderer->addText("test", "test", 350, 170, 30);
	// TODO : Remove, for testing menu only
	mRenderer->addMenu("\t\tMoveUp    W");
	mRenderer->addMenu("\t\tMoveDown  S");
	mRenderer->addMenu("\t\tMoveLeft  A");
	mRenderer->addMenu("\t\tMoveRight D");
	mRenderer->addMenu("\n\n\n");
	mRenderer->addMenu("\t\tBack\t\tLeave");

	// set texts visible
	mRenderer->setTextVisible(true);
	mRenderer->setMenuVisible(false);
    clkFruit.reset();
}

bool back = false;
bool initOnce = false;
void Game::onTick(float elapsedSeconds)
{
    ACTION(); // measure time used by this function
	if (timeout) {
		mRenderer->deleteText("score");
		mRenderer->addText("endGame", "Time Out", 150, 300, 70, RED);
		mRenderer->addText("score", "Your Score:" + std::to_string(mLevel->score), 200, 200, 30, GREEN);
	}
	if (!isStart&&!initOnce) {
		mRenderer->addText("startScene", "Tatami", 100, 300, 120, RED);
		mRenderer->addText("enterGame", "Press SPACE to start ...", 140, 200, 25, BLUE);
	}else if(isStart){		

		// key press 
		std::string keyPressed;
		if (mKeyA)
			keyPressed = "A";
		if (mKeyD)
			keyPressed = "D";
		if (mKeyW)
			keyPressed = "W";
		if (mKeyS)
			keyPressed = "S";
		if (mKeySpace)
			keyPressed = "Space";
		if (mMouseLeft) {
			keyPressed = "LeftMouse";
			mLevel->click = true;
		}

		if (!initOnce) {
			mRenderer->deleteText("startScene");
			mRenderer->deleteText("enterGame");
			mRenderer->addText("score", "Score:0", 570, 550, 30);
			auto m_trans = mLevel->getGhostObject()->getWorldTransform();
			mCamera->setRotationMatrix(to_glm(m_trans));
			auto bas = m_trans.getBasis();
			bas[2] = to_bullet(mCamera->getForwardDirection()).normalize();
			m_trans.setBasis(bas);
			mLevel->getGhostObject()->setWorldTransform(m_trans);
			initOnce = true;
		}

		// Step 1: Update the game
		if (mLevel) {
			mLevel->physicUpdate(elapsedSeconds, keyPressed);
			mLevel->update(elapsedSeconds);

		}

		// Step 2: update the camera
		auto m_trans = mLevel->getGhostObject()->getWorldTransform();
		mCamera->setPosition(to_glm(m_trans.getOrigin()));		
		btVector3 forwardDir = to_bullet(mCamera->getForwardDirection()).normalize();
		mLevel->setSword(forwardDir, btScalar(0));
		mLevel->fruitUpdate(forwardDir);		


        btScalar timeOut = 3.0;
		for (int i = 0; i < mLevel->fruitOnScene.size(); i++) {
			if (mLevel->fruitOnScene[i].timeCreated.getTimeSeconds() > timeOut)
				mLevel->removeGameObject(mLevel->fruitOnScene[i].obj);
		}
        if(clkFruit.getTimeSeconds()>timeOut){
            mLevel->removeGameObject(mLevel->findGameObject("Banana"));
            mLevel->removeGameObject(mLevel->findGameObject("Orange"));
            mLevel->removeGameObject(mLevel->findGameObject("Bomb"));
            mLevel->removeGameObject(mLevel->findGameObject("Apple"));
        }

		if (mLevel->findGameObject("Katana") && mLevel->click) {
            mLevel->setSlow = 10;
			if (sum_angle <= -M_PI_2) {
                yaw = 0.09f;
				back = true;
			}
			else if (sum_angle >= 0.0f) {
                yaw = -0.09f;
				if (back) {
					mLevel->click = false;
					back = false;
					sum_angle = 0.0f;
					mLevel->maxP = btVector3(-100, -100, -100);
					mLevel->clickonce = true;
                    mLevel->setSlow = 1;
				}
			}

			if (mLevel->click) {				
				auto katanaP = mLevel->findGameObject("Katana");
				
				btTransform t;
				mLevel->findGameObject("Katana")->getMotionState()->getWorldTransform(t);
				btMatrix3x3 orn = t.getBasis();
				orn = orn * btMatrix3x3(btQuaternion(btScalar(0.0f), btScalar(0.0f), btScalar(yaw)));
				t.setBasis(orn);
				mLevel->findGameObject("Katana")->getMotionState()->setWorldTransform(t);
				sum_angle += yaw;
				mRenderer->deleteText("score");
				mRenderer->addText("score", "Score:" + std::to_string(mLevel->score), 570, 550, 30);
			}
			else {
				mLevel->findGameObject("Katana")->getMotionState()->setWorldTransform(mLevel->swordState);
			}
		}

        if (mLevel->hitBomb/*&&mLevel->click*/)
		{
			mRenderer->addText("lose", "YOU LOSE !", 200, 250, 50, RED);
			isStart = false;
		}
		else if (mLevel->hitBomb) {
			mLevel->hitBomb = false;
		}
	}
	// Step 3: Render the game
	mRenderer->render(elapsedSeconds);
}

void Game::onResize(int w, int h)
{
    mWindowWidth = w;
    mWindowHeight = h;

    // resize camera viewport
    mCamera->resize(w, h);

    // resize renderer (targets etc.)
    mRenderer->onResize(w, h);
}

void Game::onKeyPress(int key, int scancode, int action, int modifiers)
{
    // AntTweakBar input handling
    if (action == GLFW_PRESS && TwKeyPressed(glfw2atbKey(key, scancode), glfw2atbKeyMod(modifiers)))
        return;
    if (action == GLFW_RELEASE && TwKeyTest(glfw2atbKey(key, scancode), glfw2atbKeyMod(modifiers)))
        return;

    // key pressed down
    if (action == GLFW_PRESS)
    {
        // ESC
        if (key == GLFW_KEY_ESCAPE)
        {
			//pop/close menu
			if(!popmenu)
				mRenderer->setMenuVisible(true);
			else
				mRenderer->setMenuVisible(false);
			popmenu = !popmenu;
            // close window next frame
            //glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
        }
    }

    if (action != GLFW_REPEAT)
    {
        // WASD
		
		if (key == GLFW_KEY_W) {
			mKeyW = action == GLFW_PRESS;
		}if (key == GLFW_KEY_A)
            mKeyA = action == GLFW_PRESS;
        if (key == GLFW_KEY_S)
            mKeyS = action == GLFW_PRESS;
        if (key == GLFW_KEY_D)
            mKeyD = action == GLFW_PRESS;
		if (key == GLFW_KEY_SPACE) {
			mKeySpace = action == GLFW_PRESS;
			isStart = true;
		}
    }
}

void Game::onCharInput(uint32_t codepoint)
{
    // AntTweakBar input handling
    if (TwKeyTest(codepoint, TW_KMOD_NONE))
        return;
}

void Game::onMousePress(int button, int action, int modifiers)
{
    //AntTweakBar input handling
    if (TwMouseButton(glfw2atbMouseAction(action), glfw2atbMouseButton(button)))
        return;

    // update mouse input
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        mMouseRight = action == GLFW_PRESS;
	
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		mMouseLeft = action == GLFW_PRESS;

	if (mMouseLeft&&popmenu)
	{
		if (mMouseLastX <= 850 && mMouseLastX >= 600 && mMouseLastY <= 500 && mMouseLastY >= 480)
			glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
	}

}

void Game::onMouseMove(double x, double y)
{
	// AntTweakBar input handling
	if (TwMouseMotion(x, y))
		return;

	// update last mouse pos
	if (mMouseLastX == -1)
	{
		mMouseLastX = x;
		mMouseLastY = y;
		return; // no delta data yet
	}
	// calc mouse movement
	auto dx = x - mMouseLastX;
	auto dy = y - mMouseLastY;
	mMouseLastX = x;
	mMouseLastY = y;	

	// look-around
	if (mMouseRight&&isStart)
	{
		btScalar y, p, r;
		to_bullet(mCamera->getRotationMatrix4()).getBasis().getEulerYPR(y, p, r);
		mCamera->FPSstyleLookAround(dx / mWindowWidth, dy / mWindowHeight);
		auto temp1 = to_bullet(mCamera->getForwardDirection());

		if (temp1.angle(btVector3(temp1.getX(), 0, temp1.getZ())) >= 1.3)
			mCamera->FPSstyleLookAround(-dx / mWindowWidth, -dy / mWindowHeight);		

		///get camera rotation angle
		if (mForwardLast == btVector3(0, 0, -1)){
			mForwardLast= btVector3(temp1.getX(),0, temp1.getZ());
			return;
		}

		auto temp2 = btVector3(temp1.getX(),0, temp1.getZ());
		auto rotateAngle = temp2.angle(mForwardLast);
		auto normalVec = temp2.cross(mForwardLast);
		mForwardLast = temp2;
		btVector3 forwardDir = to_bullet(mCamera->getForwardDirection()).normalize();

		/// Set player capsule direction
		auto orn = mLevel->getGhostObject()->getWorldTransform().getBasis();
		orn[2] = btVector3(forwardDir.getX(), 0, forwardDir.getZ());
		mLevel->getGhostObject()->getWorldTransform().setBasis(orn);
		

		/// setSwordRotation+Position
		if (normalVec.getY() > 0)
			mLevel->setSword(forwardDir, -rotateAngle);
		else
			mLevel->setSword(forwardDir, rotateAngle);

		/// if sword hit static object, remain at the position, else store the current transformation info
		if (!mLevel->hitStatic) {
			mLevel->currentCharacterTransform = mLevel->getGhostObject()->getWorldTransform();
			mLevel->currentCameraRotation = mCamera->getRotationMatrix3();
		}
		else {
			mLevel->getGhostObject()->setWorldTransform(mLevel->currentCharacterTransform);
			mCamera->setRotationMatrix(mLevel->currentCameraRotation);
		}

	}        
}

void Game::onMouseWheel(double xoffset, double yoffset)
{
    // AntTweakBar input handling
    if (TwMouseWheel(yoffset))
        return;
}
