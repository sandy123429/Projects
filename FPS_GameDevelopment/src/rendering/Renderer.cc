#include "Renderer.hh"

#include <aion/Action.hh>
#include <glm/vec2.hpp>

#include <glow/gl.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>
#include <glow/util/DefaultShaderParser.hh>

#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/VertexArray.hh>

#include<glow/objects/ElementArrayBuffer.hh>


#include <glow-extras/geometry/Quad.hh>

#include <Game.hh>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <logic/Level.hh>
#include <logic/GameObject.hh>

#include "Material.hh"
#include "PointLight.hh"
#include "DirectionalLight.hh"
#include "FontInterface.hh"
#include<common/bullet_helper.hh>
#include <glow/objects/Shader.hh>
#include <glow/objects/TextureCubeMap.hh>
#include <glow/data/TextureData.hh>

using namespace glow;


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};
struct PartBaseVertex {
	glm::vec2 coord;
};
struct PartInstanceVertex {
	glm::vec3 position;
	float progress;
	float random;
};
struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	float lifetime = 0;
	float maxlife = 0;
	//float random;
};


std::vector<Particle> particles;

Renderer::Renderer(Game *game) : mGame(game)
{
}


void Renderer::render(float elapsedSeconds)
{
	ACTION(); // measure time used by this function
	
	// advance time
	mTotalSeconds += elapsedSeconds;

	// get important level vars
	auto w = mGame->getWindowWidth();
	auto h = mGame->getWindowHeight();
	auto level = mGame->getLevel();
	auto cam = mGame->getCamera();


    // set the OpenGL viewport to window size
    GLOW_SCOPED(viewport, 0, 0, w, h);
	glClearColor(1, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	GLOW_SCOPED(enable, GL_DEPTH_TEST);
	GLOW_SCOPED(depthFunc, GL_LESS);
	GLOW_SCOPED(enable, GL_CULL_FACE);

	auto dlvPos = level->getPointLights().empty() ? glm::vec3(0, 1, 0) : level->getPointLights()[0]->getPosition();

	glm::mat4 shadowProjMatrix = glm::perspective(90.f, 1.0f, 0.1f, 100.f); 	//glm::mat4 shadowProjMatrix = glm::perspective(glm::pi<float>() / 1.4f, 1.0f, 3.0f, 30.0f);
	glm::mat4 shadowViewMatrix = glm::lookAt(dlvPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	
	// Draw into framebuffershadow
	{
		auto fb = mFrameBufferShadow->bind();

		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 depthMVP = shadowProjMatrix * shadowViewMatrix;

		auto shader = mShaderShadow->use();

		// loop render all object again here
		for (SharedGameObject const &obj : level->getGameObjects())
		{
			auto modelMatrix = glm::translate(obj->getPosition()) * glm::mat4(obj->getFrame());
			shader.setUniform("uDepthMVP", depthMVP * modelMatrix);

			// bind object mesh
			auto va = obj->getGpuMesh()->bind();
			va.draw();

		}
	}


	// Draw into framebuffer
	{

		auto fb = mFramebuffer->bind(); // bound until scope ends

        // clear depth and color
		GLOW_SCOPED(clearColor, 0, 0, 0, 0); //GLOW_SCOPED(clearColor, 0.00, 0.33, 0.63, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//------render background------
		renderScene();

		// enable culling and depth test
		GLOW_SCOPED(enable, GL_DEPTH_TEST);
		GLOW_SCOPED(enable, GL_CULL_FACE);

		// wireframe mode (optional)
		GLOW_SCOPED(polygonMode, GL_FRONT_AND_BACK, mWireframe ? GL_LINE : GL_FILL);

		// check if too many light sources
		if (level->getDirectionalLights().size() > 1)
			glow::error() << "More than one directional light is not implemented.";
		if (level->getPointLights().size() > 1)
			glow::error() << "More than one point light is not implemented.";

		// Draw level objects
		auto ambientColor = level->getAmbientColor();
		{
			// bind shader for the rest of this scope
			auto shader = mShaderGameObjects->use();

			// setup global shader variables
			auto viewMatrix = cam->getViewMatrix();
			auto projMatrix = cam->getProjectionMatrix();
			shader.setUniform("uViewMatrix", viewMatrix);
			shader.setUniform("uProjMatrix", projMatrix);
			shader.setUniform("uCamPos", cam->getPosition());
			shader.setTexture("uLightmapTexture", level->getLightMap());

			// light
			{
				auto dlColor = level->getDirectionalLights().empty() ? glm::vec3(0) : level->getDirectionalLights()[0]->getColor();
				auto dlDir = level->getDirectionalLights().empty() ? glm::vec3(0, 1, 0) : level->getDirectionalLights()[0]->getDirection();

 				shader.setUniform("uSunLightDir", normalize(dlDir));
                shader.setUniform("uSunLightColor", dlColor);
			}
			{
                auto plColor = level->getPointLights().empty() ? glm::vec3(0) : level->getPointLights()[0]->getColor();
                auto plPos = level->getPointLights().empty() ? glm::vec3(0, 1, 0) : level->getPointLights()[0]->getPosition();

                shader.setUniform("uPointLightPos", plPos);
                shader.setUniform("uPointLightColor", plColor);
            }

			// shadow
			glm::mat4 depthMVP = shadowProjMatrix * shadowViewMatrix;

			shader.setUniform("uDepthMVP", depthMVP);
			shader.setTexture("uTexShadow", mTextureShadow);

			// iterate over all game objects
            for (SharedGameObject const &obj : level->getGameObjects())
            {
                // bind object mesh
                auto va = obj->getGpuMesh()->bind();

                // set object-specific shader variables
                auto modelMatrix = glm::translate(obj->getPosition()) * glm::mat4(obj->getFrame());
                shader.setUniform("uModelMatrix", modelMatrix);
                shader.setUniform("uNormalMatrix", transpose(inverse(glm::mat3(modelMatrix))));
                shader.setUniform("uUsesPrecomputedLighting", obj->getUsesPrecomputedLighting());
                
				// draw once for every materials
                for (SharedMaterial const &mat : obj->getMaterials())
                {
                    // setup material-specific shader variables
                    shader.setUniform("uAmbientColor", ambientColor * mat->getAmbient());
                    shader.setUniform("uDiffuseColor", mat->getDiffuseColor());
                    shader.setUniform("uSpecularColor", mat->getSpecularColor());
                    shader.setUniform("uSpecularHardness", mat->getSpecularHardness());
                    shader.setUniform("uObjectColor", mat->getUseObjectColor() ? glm::vec3(obj->getColor()) : glm::vec3(1));
                    
					// Currently ignored: transparency
					mTexColor = mat->getTextureDiffuse().getTextureValue();
					if (mTexColor != nullptr) {
						shader.setTexture("uTextureColor", mTexColor);
					}
					mTexNormal = mat->getTextureNormal().getTextureValue();
					if (mTexNormal != nullptr) {
						shader.setTexture("uTextureNormal", mTexNormal);
					}
					mTexScale = mat->getTextureDiffuse().getScale();
					if (mTexScale != glm::vec2(0.0f, 0.0f)) {
						shader.setUniform("uTextureScale", mTexScale);
					}
					shader.setUniform("uTotalSeconds", mTotalSeconds);
					
                    // bind and draw Mesh
                    va.draw();
                }
            }

			fontInterface.DrawOverlay();

			if (mTextIsVisible) {
				std::map<std::string, Text>::iterator it;

				// Render all texts in list
				for (it = mTextsToRender.begin(); it != mTextsToRender.end(); it++) {
					Text text = it->second;
					fontInterface.PrintText2d(text.getTextString(), text.getX(), text.getY(), text.getSize(), text.getFontColor());
				}
			}

			// Render menu
			menuInterface.setMenus(mMenusToRender);
			menuInterface.PrintTextMenu();
        }
    }
		

	// Draw framebuffer to window
	// and add some simple post process

	//particle simulation
	
	{
		static float accum = 0.0;
		//static float lastTime = mTotalSeconds - elapsedSeconds;// 20.0f; // runtime; //how to get run time?
		const float timeStep = 1 / 100.0f;
		accum += elapsedSeconds;
		
		while (particles.size() < 500)
			particles.push_back({});

		while (accum > timeStep) {
			accum -= timeStep;

			for (Particle &p : particles) {
				p.lifetime += timeStep;

				if (p.lifetime >= p.maxlife) {
					p.lifetime = 0;
					p.maxlife = randf(2, 10);
					//p.random = randf(0, 1);

					p.position = glm::vec3(
						randf(-500 , 500),
						randf(-500 , 5000),
						randf(-500,  1000) //not sure of z;
					);
					
					p.velocity = glm::vec3(
						randf(-1, 1), //randf(-0.5f, 0.5f),
						randf(0, 2),
						randf(-1, 1)  //randf(-0.5f, 0.5f)
					);
				}

				p.velocity -= glm::vec3(0, 7, 0)*timeStep;
				p.position += p.velocity*timeStep;
			}
		}

		//prepare vertex data
		std::vector<PartInstanceVertex> partBatch;
		partBatch.resize(particles.size());
		for (size_t i = 0; i < particles.size(); i++) {
			auto const &p = particles[i];
			auto &pi = partBatch[i];
			pi.position = p.position;
			pi.progress = p.lifetime / p.maxlife;
			//pi.random = p.random;

		}

		abParticleInstance->bind().setData(partBatch);

	}
	{
		//black and white texture
		auto fb = mFrameBufferGlow->bind();
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);
		auto shader = mShaderGlow->use();
		shader.setTexture("uTexColor", mTexBufferColor);
		shader.setUniform("uTotalSeconds", mTotalSeconds);
		mMeshQuad->bind().draw();

	}
	{
		//vertical blur
		auto fb = mFrameBufferBlur->bind();
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);

		auto shader = mShaderBlur->use();
		// make variables and textures available
		shader.setTexture("uTexColor", mTextureGlow);
		shader.setUniform("uTotalSeconds", mTotalSeconds);
		//shader.setUniform("targetHeight", h);
		//shader.setUniform("targetWidth", w);
		shader.setUniform("isVertical", 1);
		mMeshQuad->bind().draw();
	}

	{
		//horizontal blur
		auto fb = mFrameBufferGlow->bind();
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);

		auto shader = mShaderBlur->use();
		shader.setTexture("uTexColor", mTextureBlur);
		shader.setUniform("uTotalSeconds", mTotalSeconds);
		//shader.setUniform("targetHeight", h);
		//shader.setUniform("targetWidth", w);
		shader.setUniform("isVertical", 0);
		mMeshQuad->bind().draw();
	}
	{
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		GLOW_SCOPED(disable, GL_CULL_FACE);
		auto shader = mShaderOutput->use();
		// make variables and textures available
		shader.setTexture("uTexColor", mTextureGlow);
		shader.setTexture("uTexColor2", mTexBufferColor);
		shader.setUniform("uTotalSeconds", mTotalSeconds);
		// draw fullscreen quad
		mMeshQuad->bind().draw();
	}
	
	// draw particles
	{
		GLOW_SCOPED(disable, GL_DEPTH_TEST);

		GLOW_SCOPED(enable, GL_BLEND);
		GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE); 
		
		GLOW_SCOPED(depthMask, GL_FALSE); 

		GLOW_SCOPED(disable, GL_CULL_FACE); // other solution: LookAt

		auto shader = mShaderParticle->use();
	
		glm::mat4 modelMatrix = glm::mat4();
		glm::mat4 projMatrix = cam->getProjectionMatrix();
		glm::mat4 viewMatrix = cam->getViewMatrix();
		glm::mat4 modelViewProj = projMatrix * viewMatrix * modelMatrix;
		glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;

		shader.setUniform("uModelViewMatrix", modelViewMatrix);
		shader.setUniform("uProjectionMatrix", modelViewProj);

		shader.setTexture("uTexColor", mParticleTexture);
		mParticles->bind().draw(particles.size());

		GLOW_SCOPED(disable, GL_BLEND);
		GLOW_SCOPED(depthMask, GL_TRUE);
	}
}

//-------a random generator--------------
float Renderer::randf(float min, float max) {
	return min + rand() / float(RAND_MAX)*(max - min);
}

//-----------render background-------------
void Renderer::renderScene() {
	{	//bg
		GLOW_SCOPED(disable, GL_DEPTH_TEST);
		auto shader = mShaderBG->use();

		shader.setTexture("uTexture", mBackgroundTexture);
		auto cam = mGame->getCamera();
		auto invProj = inverse(cam->getProjectionMatrix());
		auto invView = inverse(cam->getViewMatrix());
		shader.setUniform("uInvProj", invProj);
		shader.setUniform("uInvView", invView);

		mQuadBG->bind().draw();
	}

}


void Renderer::onInit()
{
	ACTION(); // measure time used by this function

	// Base folder for shaders
	// (working director is bin/)
	glow::DefaultShaderParser::addIncludePath("shader");

	// Common meshes
	mMeshQuad = geometry::Quad<>().generate();

	// Framebuffer
	mTargets.push_back(mTexBufferColor = TextureRectangle::create(1, 1, GL_RGB16F));            // 16 bit RGB
	mTargets.push_back(mTexBufferDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32)); // 32 bit depth
	mFramebuffer = Framebuffer::create(
	{
		{ "fColor", mTexBufferColor } // out vec3 fColor;
	},
		mTexBufferDepth);

	// Blur buffer
	mTargets.push_back(mTextureBlur = TextureRectangle::create(1, 1, GL_RGB16F));            // 16 bit RGB

	mFrameBufferBlur = Framebuffer::create(
	{
		{ "fColor", mTextureBlur } // out vec3 fColor;
	},
	{});

	mTargets.push_back(mTextureGlow = TextureRectangle::create(1, 1, GL_RGB16F));            // 16 bit RGB

	mFrameBufferGlow = Framebuffer::create(
	{
		{ "fColor", mTextureGlow } // out vec3 fColor;
	},
	{});

	// ----------Fragmentbuffershadow-------
	mTextureShadow = TextureRectangle::create(shadowSize, shadowSize, GL_DEPTH_COMPONENT32);
	//mTargets.push_back(mTextureShadow);
	mTextureShadow->bind().setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	mFrameBufferShadow = Framebuffer::create(
	{
		//framebuffer without color attachment
	},
	mTextureShadow);

	// Shader
	// ... output shader, automatically detects and loads bin/shader/output.fsh and ~.vsh
	mShaderOutput = Program::createFromFile("output");
	// ... object shader, using generic vertex shader and specific fragment shader
	mShaderGameObjects = Program::createFromFiles({ "objects/generic.vsh",
		"objects/game-object.fsh" });
	// ... shadow shder, using the same with game-object
	mShaderShadow = Program::createFromFiles({ "objects/shadow.vsh",
		"objects/shadow.fsh" });
	// ... particle shader
	mShaderParticle = Program::createFromFiles( {"objects/particle.vsh", "objects/particle.fsh"});

	//BLUR SHADER
	mShaderBlur = Program::createFromFiles({ "blur/blur.vsh",
		"blur/blur.fsh" });
	//glow shader
	mShaderGlow = Program::createFromFiles({ "glow/glow.vsh",
		"glow/glow.fsh" });

	auto pbtParticle = util::pathOf(__FILE__);
	mParticleTexture = Texture2D::createFromData(TextureData::createFromFile(pbtParticle + "/cherry.jpg"));

	abParticleBase = ArrayBuffer::create();
	abParticleInstance = ArrayBuffer::create();

	// Particle setup
	PartBaseVertex pvertices[6] = {
		{ { 0, 0 } },
		{ { 1, 0 } },
		{ { 1, 1 } },

		{ { 0, 0 } },
		{ { 1, 1 } },
		{ { 0, 1 } }
	};

	//first define, then bind
	abParticleBase->defineAttribute(&PartBaseVertex::coord, "aQuadCoord");
	abParticleBase->bind().setData(pvertices); 

	abParticleInstance->defineAttribute(&PartInstanceVertex::position, "aPosition", AttributeMode::Float, 1U); 
	abParticleInstance->defineAttribute(&PartInstanceVertex::progress, "aProgress", AttributeMode::Float, 1U);
	abParticleInstance->defineAttribute(&PartInstanceVertex::random, "aRandom", AttributeMode::Float, 1U); 

	mParticles = VertexArray::create({ abParticleBase, abParticleInstance }, nullptr, GL_TRIANGLES); 


	// ... background shader
	mShaderBG = Program::createFromFile("bg");
	mQuadBG = geometry::Quad<>().generate();

	auto pbt = util::pathOf(__FILE__) + "/cubemaps/chapel/";
	//auto pbt = "/../level/test.blend.data/cubemaps/chapel/";
	mBackgroundTexture = TextureCubeMap::createFromData(TextureData::createFromFileCube(
		pbt + "/posx.jpg",                                                              //
		pbt + "/negx.jpg",                                                              //
		pbt + "/posy.jpg",                                                              //
		pbt + "/negy.jpg",                                                              //
		pbt + "/posz.jpg",                                                              //
		pbt + "/negz.jpg"                                                               //
	));

	// ... load font interface
	auto fontDir = util::pathOf(__FILE__) + "/font/";
	fontInterface.InitText2d(fontDir + "ShowcardGothic.png");

	// ... load menu interface
	menuInterface.Init(&fontInterface);



    // Register variables to anttweakbar
    //auto twb = mGame->getTweakBar();
    //TwAddVarRW(twb, "Draw Wireframe", TW_TYPE_BOOLCPP, &mWireframe, "group=rendering");
}

void Renderer::onLevelLoaded(const SharedLevel &level)
{
	ACTION(); // measure time used by this function
}

void Renderer::onResize(int w, int h)
{
	ACTION(); // measure time used by this function

			  // resize all fullscreen textures
	for (auto const &target : mTargets)
		target->bind().resize(w, h);
}

void Renderer::addText(std::string key, std::string textString, int x, int y, int size, Color fontColor) {
	Text * newText = new Text(textString, x, y, size, fontColor);
	mTextsToRender.insert(std::pair<std::string, Text>(key, *newText));
}

void Renderer::deleteText(std::string key)
{
	std::map<std::string, Text>::iterator toDelete;
	toDelete = mTextsToRender.find(key);
	mTextsToRender.erase(toDelete);
}

void Renderer::addMenu(std::string menuString)
{
	mMenusToRender.push_back(menuString);
}

void Renderer::setMenuVisible(bool isVisible)
{
	menuInterface.setIsVisible(isVisible);
}

void Renderer::setTextVisible(bool isVisible)
{
	mTextIsVisible = isVisible;
}
