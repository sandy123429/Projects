#include "Level.hh"
#include "Mesh.hh"
#include <stdlib.h>
#include <time.h>
#include <glow/common/log.hh>

#include <aion/Action.hh>

#include <common/assert.hh>
#include <common/bullet_helper.hh>

#include <Game.hh>

#include <rendering/Material.hh>

#include "GameObject.hh"
#include<AntTweakBar.h>
#include <btBulletDynamicsCommon.h>
#include<BulletDynamics/Character/btKinematicCharacterController.h>
#include<BulletCollision/CollisionDispatch/btGhostObject.h>
#define _USE_MATH_DEFINES
#include<math.h>
struct ContactSensorCallback : public btCollisionWorld::ContactResultCallback {

	//! Constructor, pass whatever context you want to have available when processing contacts
	/*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
	*  (supplied by the superclass) for needsCollision() */
	ContactSensorCallback(btRigidBody& tgtBody, SharedGameObject context, bool& hitStatic, bool& hitFruit, bool& hitBomb, btVector3& intersectPt)
		: btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxt(context),hitStatic(hitStatic), hitFruit(hitFruit), hitBomb(hitBomb), intersectPt(intersectPt) { }

	btRigidBody& body; 
	SharedGameObject ctxt;
	bool& hitStatic;
	bool& hitFruit;
	bool& hitBomb;
	btVector3& intersectPt;
	//! If you don't want to consider collisions where the bodies are joined by a constraint, override needsCollision:
	/*! However, if you use a btCollisionObject for #body instead of a btRigidBody,
	 *  then this is unnecessary — checkCollideWithOverride isn't available */
	virtual bool needsCollision(btBroadphaseProxy* proxy) const {
		// superclass will check m_collisionFilterGroup and m_collisionFilterMask
		if (!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
			return false;
		// if passed filters, may also want to avoid contacts between constraints
		return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
	}

	//! Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
	virtual btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* colObj0, int partId0, int index0,
		const btCollisionObjectWrapper* colObj1, int partId1, int index1)
	{
		btVector3 pt; // will be set to point of collision relative to body
		if (colObj0->m_collisionObject == &body) {
			pt = cp.m_localPointA;
			intersectPt = pt;
		}
		else {
			assert(colObj1->m_collisionObject == &body && "body does not match either collision object");
			pt = cp.m_localPointB;
			intersectPt = pt;
		}
		if (colObj0->getCollisionShape()->getUserIndex() == 1)
			intersectPt = cp.m_localPointA;
		else
			intersectPt = cp.m_localPointB;
		if (ctxt->getTag()=="Static"&&(colObj0->getCollisionObject()->isKinematicObject() || colObj1->getCollisionObject()->isKinematicObject())) {
            hitStatic = true;
        }else if(ctxt->getTag()=="Fruit" && (colObj0->getCollisionObject()->isKinematicObject() || colObj1->getCollisionObject()->isKinematicObject())){
			hitFruit = true;
		}
		else if (ctxt->getTag() == "Bomb" && (colObj0->getCollisionObject()->isKinematicObject() || colObj1->getCollisionObject()->isKinematicObject())) {
			hitBomb = true;
        }

		return 0; // There was a planned purpose for the return value of addSingleResult, but it is not used so you can ignore it.
	}
};



Level::Level(Game* game) : mGame(game)
{
    // create bullet physics
    mBulletBroadphase = new btDbvtBroadphase;
    mBulletCollisionConfig = new btDefaultCollisionConfiguration;
    mBulletCollisionDispatcher = new btCollisionDispatcher(mBulletCollisionConfig);
	mBulletSolver = new btSequentialImpulseConstraintSolver;
    mBulletWorld = new btDiscreteDynamicsWorld(mBulletCollisionDispatcher, mBulletBroadphase, mBulletSolver, mBulletCollisionConfig);
	mBulletWorld->setGravity(btVector3(0, -10, 0)); // set initial gravity
	setSlow = 1;	
	
}

Level::~Level()
{
    // cleanup physics
    for (auto const& obj : mGameObjects)
        if (obj->getRigidBody())
            mBulletWorld->removeRigidBody(obj->getRigidBody());
	delete mBulletWorld;
	delete mBulletSolver;
	delete mBulletCollisionConfig;
	delete mBulletCollisionDispatcher;
	delete mBulletBroadphase;
}

void Level::update(float elapsedSeconds)
{
    ACTION(); // measure time used by this function

    // Update physics
    mBulletWorld->stepSimulation(elapsedSeconds/setSlow, 10);

    // Update objects
	hitStatic = false;
	
    for (auto const& obj : mGameObjects)
    {
        // physics
        if (obj->getRigidBody())
        {
            // copy transformation to object (sync physics and rendering)
            btTransform t;
			hitFruit = false;
            obj->getMotionState()->getWorldTransform(t);
            obj->setTransformation(to_glm(t));
			ContactSensorCallback callback(*obj->getRigidBody(), obj, hitStatic, hitFruit, hitBomb, intersectPt);
			mBulletWorld->contactTest(obj->getRigidBody(), callback);
			if (hitFruit&&click&&clickonce) {
				score += 3; 
				std::vector<glm::vec3> plane;
				auto katanaP = findGameObject("Katana");
				auto point = obj->getRigidBody()->getCenterOfMassTransform().inverse()*katanaP->getRigidBody()->getWorldTransform().getOrigin();
				plane.push_back(to_glm(point));
                plane.push_back(to_glm(intersectPt + btVector3(0, -3, 0)));
				plane.push_back(to_glm(intersectPt));

				/// create cut mesh
				for (Mesh m : obj->getCpuMesh()->cutMesh(*obj->getCpuMesh(), plane, true)) {
					auto tri = m.getTriangles();
					auto mesh = std::make_shared<Mesh>(m);
					btConvexShape *tmpshape = new btConvexTriangleMeshShape(createConvexTriangleMeshShape(tri));
					btCollisionShape* btShape = tmpshape;
					saveSplitObjects.push_back(saveSplitObj(obj, obj->getRigidBody()->getWorldTransform().getOrigin(), obj->getName() + "_c_" + std::to_string(splitNum++), mesh, btShape, "split", btVector3(0, 0, 0)));
				}
				delSplitedObject.push_back(obj);
				clickonce = false;					
			}
        }		
    }	
	
	
	for (auto const& obj : saveSplitObjects)
		createGameObject(obj.name, obj.obj, obj.origin, obj.mesh, obj.btShape, obj.typ, btVector3(0, 0, 0));
	saveSplitObjects.clear();

	for (auto const& obj : delSplitedObject)
		removeGameObject(obj);
	delSplitedObject.clear();
}


//update physic every tick
void Level::physicUpdate(float elapsedSeconds, std::string keyPressed)
{ 
	btTransform xform;
	xform = mGhostObject->getWorldTransform();

	btVector3 forwardDir = xform.getBasis()[2];
	btVector3 upDir = xform.getBasis()[1];
	btVector3 strafeDir = xform.getBasis()[0];
	forwardDir.normalize();

	btVector3 walkDirection = btVector3(0.0, 0.0, 0.0);
	btScalar walkVelocity = 2;
	btScalar walkSpeed = walkVelocity* elapsedSeconds;

	btVector3 horizontalDir = forwardDir.cross(upDir).normalize();

	if (keyPressed == "D")
		walkDirection += horizontalDir;

	if (keyPressed == "A")
		walkDirection -= horizontalDir;

	if (keyPressed == "W")
		walkDirection += forwardDir;

	if (keyPressed == "S")
		walkDirection -= forwardDir;

	if (keyPressed == "Space")
		m_character->jump();


	if (walkDirection == btVector3(0, 0, 0)) {
		walkDirection == btVector3(0, 0, 1);
	}
	m_character->setWalkDirection(walkDirection*walkSpeed);

	if (walkDirection == btVector3(0.0, 0.0, 0.0))
		walkDirection = btVector3(0.0, 0.0, 1.0);

	if (!hitStatic)
		currentCharacterTransform = mGhostObject->getWorldTransform();
	else
		mGhostObject->setWorldTransform(currentCharacterTransform);
}

void Level::fruitUpdate(btVector3 forwardDir)
{

	for (fruitInfo obj : fruitOnScene) {
		float v1 = rand() % 20 -10;
		float v2 = rand() % 20 -10;
		float v3 = rand() % 2;

		if(v3==1.0)
            obj.obj->getRigidBody()->applyCentralImpulse(btVector3(-3.0, 0.0f,3.0f)/10);
		if (v3 == 0.0)
            obj.obj->getRigidBody()->applyCentralImpulse(btVector3(3.0, 0.0f,3.0f)/10);

        btMatrix3x3 ornRot = obj.obj->getRigidBody()->getWorldTransform().getBasis();
        ornRot = ornRot*btMatrix3x3(btQuaternion(btVector3(3.0f, 2.0f, 1.0f), 0.1));
        obj.obj->getRigidBody()->getWorldTransform().setBasis(ornRot);
	}
}


void Level::loadFruit(btVector3 forwardDir) {
	btTransform t;
	findGameObject("Katana")->getMotionState()->getWorldTransform(t);
	btVector3 start = t.getOrigin();
    btVector3 end = start + forwardDir * 3 + btVector3(-1,1,0);
	btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
	mBulletWorld->rayTest(start, end, RayCallback);
	if (RayCallback.hasHit()) {
		//end = RayCallback.m_hitPointWorld;
		//btVector3 normal = RayCallback.m_hitNormalWorld;
	}
	else {

		///generate fruit on scene
		srand(time(NULL));
		if (fruitsForGenerate.size() != 0) {
			SharedGameObject o = fruitsForGenerate[rand() % fruitsForGenerate.size()].obj;
			btCollisionShape* btshape = nullptr;
			if (o->getCollisionShape()->getName() == "SPHERE")
			{
				auto radius = o->getCpuMesh()->getCollisionSphereRadius();
				btshape = new btSphereShape(radius);
			}
			else if (o->getCollisionShape()->getName() == "CylinderY")
			{
				auto halfExtends = o->getCpuMesh()->getCollisionBoxHalfExtends();
				btshape = new btCylinderShape(to_bullet(halfExtends));
			}
			else if (o->getCollisionShape()->getName() == "ConvexTriMesh")
			{
				auto trimesh = createConvexTriangleMeshShape(o->getCpuMesh()->getTriangles());
				btConvexShape *tmpshape = new btConvexTriangleMeshShape(trimesh);
				btshape = tmpshape;
			}
			else {
				btshape = new btSphereShape(1);
			}
			createGameObject(o->getName() + std::to_string(fruitCount++), o, end, o->getCpuMesh(), btshape, o->getTag(), forwardDir);
		}
	}
}


void Level::setSword(btVector3 forwardDir, btScalar angle)
{
	if (findGameObject("Katana")) {
        auto obj = findGameObject("Katana");
		auto m_trans = mGhostObject->getWorldTransform();
		btTransform t;
		obj->getMotionState()->getWorldTransform(t);
		t.setOrigin(m_trans.getOrigin());
		btMatrix3x3 orn = t.getBasis();
		orn *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), angle));
		t.setBasis(orn);
		t.setOrigin(m_trans.getOrigin() + forwardDir.normalized()*2);
		swordState = t;
		obj->getMotionState()->setWorldTransform(t);
	}
}

/// create GameObject from source
void Level::createGameObject(std::string name, SharedGameObject src, btVector3 origin, SharedMesh mesh, btCollisionShape* btShape, std::string typ, btVector3 forwardDir)
{
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(origin);
	trans.setBasis(swordState.getBasis());
	btScalar mass = btScalar(1.0f);
	auto motionState = new btDefaultMotionState(trans);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, btShape);
	auto rb = new btRigidBody(info);
	//rb->setGravity(btVector3(0,-10,0));
			
	auto obj = std::make_shared<GameObject>(name);
	obj->setTag(typ);
	obj->setColor(src->getColor());
	obj->setCpuMesh(mesh);
	obj->setFrame(src->getFrame());
	obj->setGpuMesh(mesh->uploadMesh());
	obj->setMaterials(src->getMaterials());
	obj->setMotionState(motionState);
	obj->setCollisionShape(btShape);
	obj->setRigidBody(rb);
	addGameObject(obj);

    if (obj->getTag()=="Fruit"|| obj->getTag() == "Bomb")
		fruitOnScene.push_back(obj);
    if (obj->getTag() == "split")
        splitOnScene.push_back(obj);
}

///create convext collsionshape
btTriangleMesh* Level::createConvexTriangleMeshShape(std::vector<Mesh::Triangle> tri)
{
	auto trimesh = new btTriangleMesh();
	for (Mesh::Triangle t : tri) {
		btVector3 vert0 = to_bullet(t.vertices[0].position);
		btVector3 vert1 = to_bullet(t.vertices[1].position); 
		btVector3 vert2 = to_bullet(t.vertices[2].position); 
		trimesh->addTriangle(vert0, vert1, vert2);
	}
	return trimesh;
}


void Level::onInit()
{
    ACTION(); // measure time used by this function
}

//setup the character controller
void Level::setupCharacterController()
{
	btTransform startTransform;
	startTransform.setIdentity();

	//startTransform.setOrigin(btVector3(0, 1, 0));
    startTransform.setOrigin(btVector3(5, 1, -5));

	mGhostObject = new btPairCachingGhostObject();
	mGhostObject->setWorldTransform(startTransform);
	mBulletWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

	btScalar characterHeight = 3;
	btScalar characterWidth = 0.5;
	btConvexShape* capsule = new btCapsuleShape(characterWidth, characterHeight);
	mGhostObject->setCollisionShape(capsule);
	mGhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	btScalar stepHeight = btScalar(0.5);
	m_character = new btKinematicCharacterController(mGhostObject, capsule, stepHeight);
	mBulletWorld->addCollisionObject(mGhostObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	mBulletWorld->addAction(m_character);
	mGhostObject->getWorldTransform().setBasis(btMatrix3x3(btQuaternion(btVector3(0, 1, 0), 0)));
}

void Level::addGameObject(const SharedGameObject& obj)
{
    mGameObjects.push_back(obj);

	if (obj->getRigidBody()) {
		mBulletWorld->addRigidBody(obj->getRigidBody());	
		if(obj->getTag()=="Fruit")
			obj->getCollisionShape()->setUserIndex(1);
		else
			obj->getCollisionShape()->setUserIndex(0);
	}	
}

void Level::removeGameObject(const SharedGameObject& obj)
{
    for (auto it = begin(mGameObjects); it != end(mGameObjects); ++it)
        if (*it == obj) // found
        {
            if (obj->getRigidBody())
                mBulletWorld->removeRigidBody(obj->getRigidBody());
            mGameObjects.erase(it);
            return;
        }
}

void Level::addMaterial(const SharedMaterial& mat)
{
    gdassert(!mMaterials.count(mat->getName()), "Material already registered");

    mMaterials[mat->getName()] = mat;
}

//Find GameObject with name
SharedGameObject Level::findGameObject(std::string name)
{
	for (SharedGameObject obj : mGameObjects) {
		if (obj->getName() == name)
			return obj;
	}
	return NULL;
}
