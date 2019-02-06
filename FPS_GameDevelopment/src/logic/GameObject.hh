#pragma once

#include <string>
#include <vector>
#include <map>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <glow/common/property.hh>
#include <glow/common/shared.hh>
#include <glow/fwd.hh>

class btCollisionShape;
class btRigidBody;
struct btDefaultMotionState;

GLOW_SHARED(class, Material);
GLOW_SHARED(class, Mesh);

class GameObject
{
private: // members
    /// Object name
    std::string mName;
	std::string mTag;
    /// Object position
    glm::vec3 mPosition;
    /// Object coordinate frame (includes rotation and scale)
    glm::mat3 mFrame;

    /// Object color with alpha
    glm::vec4 mColor = {1, 1, 1, 1};

    /// List of materials
    std::vector<SharedMaterial> mMaterials;

    /// Collision shape
    btCollisionShape* mCollisionShape = nullptr;
    /// Physics body
    btRigidBody* mRigidBody = nullptr;
    /// Physics motion state
    btDefaultMotionState* mMotionState = nullptr;

    /// CPU Mesh
    SharedMesh mCpuMesh;
    /// OpenGL mesh
    glow::SharedVertexArray mGpuMesh;

    /// Whether object uses precomputed lighting
    bool mUsesPrecomputedLighting = true;

    /// Custom properties
    std::map<std::string, float> mCustomProperties;

public: // getter, setter
	GLOW_PROPERTY(Name);
	GLOW_PROPERTY(Tag);
    GLOW_PROPERTY(Position);
    GLOW_PROPERTY(Frame);
    GLOW_PROPERTY(Color);
    GLOW_PROPERTY(GpuMesh);
    GLOW_PROPERTY(CpuMesh);
    GLOW_PROPERTY(Materials);
    GLOW_PROPERTY(CustomProperties);

    GLOW_PROPERTY(CollisionShape);
    GLOW_PROPERTY(RigidBody);
    GLOW_PROPERTY(MotionState);

    GLOW_PROPERTY(UsesPrecomputedLighting);

    glm::mat4 getTransformation() const;
    void setTransformation(glm::mat4 const& m);
public: // functions
    GameObject(std::string const& name);

    ~GameObject();
};
