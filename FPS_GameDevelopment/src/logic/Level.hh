#pragma once

#include <vector>
#include <map>
#include <string>

#include <glow/common/shared.hh>
#include <glow/common/property.hh>
#include <glow/fwd.hh>

#include <glm/vec3.hpp>
#include <glm/glm.hpp>

#include<BulletDynamics/Character/btKinematicCharacterController.h>
#include<BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include "Mesh.hh"

GLOW_SHARED(class, Level);
GLOW_SHARED(class, Material);
GLOW_SHARED(class, GameObject);
GLOW_SHARED(class, PointLight);
GLOW_SHARED(class, DirectionalLight);
GLOW_SHARED(class, Mesh);

class Game;

// bullet forward declarations
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btDiscreteDynamicsWorld;
class btSequentialImpulseConstraintSolver;
class btPairCachingGhostObject;
class btKinematicCharacterController;

/**
 * @brief The Level contains all objects and logic for a single game level
 */
class Level
{
private: // member

    /// Backreference to the game
    Game* const mGame;

    /// Name of the file from which the level was loaded
    std::string mFileName;

    /// Ambient color
    glm::vec3 mAmbientColor;

    /// Mapping from material name to material
    std::map<std::string, SharedMaterial> mMaterials;

    /// Custom property map
    std::map<std::string, float> mCustomProperties;

    /// List of all game objects
    std::vector<SharedGameObject> mGameObjects;

    /// List of all point lights
    std::vector<SharedPointLight> mPointLights;

    /// List of all directional lights
    std::vector<SharedDirectionalLight> mDirectionalLights;

    /// Light map texture for pre-computed global illumination
    glow::SharedTexture2D mLightMap;
	
private: // bullet physics

    btBroadphaseInterface* mBulletBroadphase = nullptr;
    btDefaultCollisionConfiguration* mBulletCollisionConfig = nullptr;
    btCollisionDispatcher* mBulletCollisionDispatcher = nullptr;
    btSequentialImpulseConstraintSolver* mBulletSolver = nullptr;
    btDiscreteDynamicsWorld* mBulletWorld = nullptr;
	btPairCachingGhostObject* mGhostObject;
	btKinematicCharacterController* m_character;	

public: // getter, setter

    GLOW_GETTER(Game);
    GLOW_GETTER(FileName);
    GLOW_GETTER(Materials);
    GLOW_GETTER(CustomProperties);
    GLOW_GETTER(PointLights);
    GLOW_GETTER(DirectionalLights);
    GLOW_GETTER(GameObjects);
    GLOW_GETTER(LightMap);
	GLOW_GETTER(GhostObject);
    GLOW_PROPERTY(AmbientColor);
	
public: //parameters

	int setSlow = 1;
	btVector3 maxP = btVector3(-100, -100, -100);
	btVector3 upK = btVector3(0, 1, 0);
	btVector3 intersectPt;
	btTransform swordState;

	struct saveSplitObj {
		SharedGameObject obj;
		btVector3 origin;
		SharedMesh mesh;
		btCollisionShape* btShape;
		std::string name;
		btVector3 forwardDir;
		std::string typ;
		saveSplitObj(SharedGameObject b, btVector3 origin, std::string name, SharedMesh mesh, btCollisionShape* btShape, std::string typ, btVector3 forwardDir)
			: obj(b), origin(origin), name(name), mesh(mesh), btShape(btShape), typ(typ), forwardDir(forwardDir) {}
	};
	std::vector<saveSplitObj> saveSplitObjects;
	std::vector<SharedGameObject> delSplitedObject;

	struct fruitForGenerate {
		SharedGameObject obj;
		int id;
		fruitForGenerate(SharedGameObject b, int id): obj(b), id(id){}
	};
	std::vector<fruitForGenerate> fruitsForGenerate;

	struct fruitInfo {
		SharedGameObject obj;
		btClock timeCreated;
		fruitInfo(SharedGameObject b) : obj(b) {
			timeCreated.reset();
		}
	};


	std::vector<fruitInfo> fruitOnScene;
	std::vector<SharedGameObject> splitOnScene;
	
	///hit static wall/table = true
	bool hitStatic = false;

	///hit fruit = true
	bool hitFruit = false;

	///if hit bomb  = true
	bool hitBomb = false;

	///if mouse left click = true
	bool click = false;

	///if click one once = true;
	bool clickonce = true;

	int splitNum = 1;
	int score = 0;
	int fruitCount=0;
	int fruitNum = 0;

	btTransform currentCharacterTransform;
	glm::mat3 currentCameraRotation;

public: // public functions

    /// ctor
    Level(Game* game);
    /// dtor
    ~Level();

    /// updates level logic
    void update(float elapsedSeconds);
	void physicUpdate(float elapsedSeconds, std::string keyPressed);
	void fruitUpdate(btVector3 forwardDir);
	void loadFruit(btVector3 forwardDir);
    /// loads this level from a .blend file
    /// (uses exported file if newer, otherwise automatically invokes the blender exporter)
    void loadFromFile(std::string const& blendFileName);
	
	///setup character controller
	void setupCharacterController();

    /// Adds a new material to the material map
    /// CAUTION: name must be unique
    void addMaterial(SharedMaterial const& mat);

	///create a convec triangle mesh shape
	btTriangleMesh* createConvexTriangleMeshShape(std::vector<Mesh::Triangle> tri);
	
	///create a game object from source
	void createGameObject(std::string name, SharedGameObject src, btVector3 origin, SharedMesh mesh, btCollisionShape* btShape, std::string typ, btVector3 fowardDir);
	
	/// Find GameObject with name
	SharedGameObject findGameObject(std::string name);
	
	///set sword position and rotation
	void setSword(btVector3 forwardDir, btScalar angle);
	
public: // events

    /// Called after a level was loaded and before it's updated/rendered for the first time
    void onInit();	/// Adds a game object to the level
    /// Also adds the object to the physics if needed
    void addGameObject(SharedGameObject const& obj);
    /// Removes a game object from the level
    /// Also removes the object from the physics if needed
    void removeGameObject(SharedGameObject const& obj);
};
