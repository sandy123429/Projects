#include "GameObject.hh"

#include <btBulletDynamicsCommon.h>

glm::mat4 GameObject::getTransformation() const
{
    glm::mat4 m(mFrame);
    m[3] = glm::vec4(mPosition, 1.0f);
    return m;
}

void GameObject::setTransformation(const glm::mat4 &m)
{
    mFrame = glm::mat3(m);
    mPosition = glm::vec3(m[3][0], m[3][1], m[3][2]);
}

GameObject::GameObject(const std::string &name) : mName(name)
{
}

GameObject::~GameObject()
{
    // cleanup physics
	delete mCollisionShape;
    delete mRigidBody;
    delete mMotionState;
}
