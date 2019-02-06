#include <iostream>
#include <chrono>
#include <cmath>

// glow BEFORE glfw (due to ogl conflicts)
#include <glow/gl.hh>
#include <glow/glow.hh>

#include <GLFW/glfw3.h>

#include <AntTweakBar.h>

#include <aion/ActionAnalyzer.hh>

#include "Game.hh"
#include <logic/Level.hh>
#include <rendering/Renderer.hh>
#include<common/bullet_helper.hh>

// Taken from http://www.glfw.org/documentation.html
int main(int argc, char **argv)
{
    const int initialWidth = 1440;
    const int initialHeight = 720;

    // Initialize the library
    if (!glfwInit())
    {
        std::cerr << "Unable to initialize GLFW" << std::endl;
        return -1;
    }

    // Request debug context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Create a windowed mode window and its OpenGL context
    auto window = glfwCreateWindow(initialWidth, initialHeight, "GLFW Window", NULL, NULL);
    if (!window)
    {
        std::cerr << "Unable to create a GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current (now we can use OpenGL)
    glfwMakeContextCurrent(window);

    // Initialize GLOW
    if (!glow::initGLOW())
    {
        std::cerr << "Unable to initialize GLOW" << std::endl;
        return -1;
    }

    // restore ogl state
    glow::restoreDefaultOpenGLState();

    // create game
{
    Game game;
    glfwSetWindowTitle(window, game.getName().c_str());

    

    // register GLFW callbacks
    glfwSetWindowSizeCallback(window, [](GLFWwindow *, int w, int h)
                              {
                                  TwWindowSize(w, h);
                                  Game::the()->onResize(w, h);
                              });
    glfwSetKeyCallback(window, [](GLFWwindow *, int key, int scancode, int action, int mods)
                       {
                           Game::the()->onKeyPress(key, scancode, action, mods);
                       });
    glfwSetCharCallback(window, [](GLFWwindow *, uint32_t code)
                        {
                            Game::the()->onCharInput(code);
                        });
    glfwSetMouseButtonCallback(window, [](GLFWwindow *, int button, int action, int mods)
                               {
                                   Game::the()->onMousePress(button, action, mods);
                               });
    glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y)
                             {
                                 Game::the()->onMouseMove(x, y);
                             });
    glfwSetScrollCallback(window, [](GLFWwindow *, double x, double y)
                          {
                              Game::the()->onMouseWheel(x, y);
                          });

	// initialize game
	game.onInit(window, argc, argv);

    // trigger first resize
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    game.onResize(width, height);

    // vsync
    glfwSwapInterval(1);

    // Loop until the user closes the window
    int frames = 0;
    double totalTime = 0;
	double gameTime = 90;
    double lastTime = glfwGetTime();
	double fruitupdateTimer=1;
	double splitupdateTimer = 1;

	bool isOnce = false;
	std::string gameTimer;
	std::string lastTimer = "";
    while (!glfwWindowShouldClose(window))
    {
		auto now = std::chrono::system_clock::now();

		// Poll for and process events
		glfwPollEvents();

		// Advance time
		auto newTime = glfwGetTime();

		auto elapsedSeconds = fmax(newTime - lastTime, 1 / 10000.); // at least 0.1ms elapsed
		lastTime = newTime;
			
		// Updates and renders the game
		game.onTick(elapsedSeconds);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// update timing
		auto end = std::chrono::system_clock::now();
		double secs = std::chrono::duration<double>(end - now).count();
		if (game.isStart) {
			totalTime += secs;
			//calculate game time according to physic slowdown
			gameTime -= secs / game.getLevel()->setSlow;
			fruitupdateTimer -= secs;
			gameTimer = "time:" + std::to_string(int(gameTime)) + "s";
			if (lastTimer == "") {
				game.getRenderer()->addText("time", gameTimer, 10, 550, 30);
			}
			else {
				game.getRenderer()->deleteText("time");
				game.getRenderer()->addText("time", gameTimer, 10, 550, 30);
			}
			lastTimer = gameTimer;
			++frames;
		}

		if (gameTime <= 0) {
			game.isStart = false;
			game.timeout = true;				
		}

		if (fruitupdateTimer <= 0) {
			game.getLevel()->loadFruit(to_bullet(game.getCamera()->getForwardDirection()).normalize());
			fruitupdateTimer = 1;
		}

		if (game.getLevel()->splitOnScene.size() > 0) {
			splitupdateTimer -= secs;
			if (splitupdateTimer <= 0) {
				splitupdateTimer = 1;
				auto o = game.getLevel()->splitOnScene[0];
				game.getLevel()->removeGameObject(o);
				game.getLevel()->splitOnScene.erase(game.getLevel()->splitOnScene.begin());
			}
		}
    }
}

    glfwTerminate();

    // aion dump
    aion::ActionAnalyzer::dumpSummary(std::cout, false);

    return 0;
}
