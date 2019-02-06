#include "Level.hh"

#include <fstream>

#include <common/assert.hh>
#include <common/bullet_helper.hh>

#include <aion/Action.hh>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glow/common/file_utils.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/common/str_utils.hh>
#include <glow/common/log.hh>

#include <glow/data/SurfaceData.hh>
#include <glow/data/TextureData.hh>

#include <glow/objects/Texture2D.hh>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/timing/PerformanceTimer.hh>

#include <common/jsonxx.hh>

#include <glm/glm.hpp>

#include <rendering/Material.hh>
#include <rendering/PointLight.hh>
#include <rendering/DirectionalLight.hh>
#include <rendering/TextureMaterial.hh>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

#include <gi/computeGI.hh>

#include <typeinfo>
#include "math.h"
#include "GameObject.hh"
#include "Mesh.hh"

std::string remove_char(std::string str, std::string ch)
{
	// remove all occurrences of char ch from str
	std::string::size_type i = str.find(ch);
	if (i != std::string::npos)
		str.erase(i, ch.length());
	return str;
}

static glm::vec2 jsonVec2(jsonxx::Array const& values)
{
    if (values.size() != 2 || !values.has<jsonxx::Number>(0) || !values.has<jsonxx::Number>(1))
    {
        glow::error() << "Expected 2 numeric values for 2D vector, got " << values.json();
        return {};
    }

    return {values.get<jsonxx::Number>(0), values.get<jsonxx::Number>(1)};
}
static glm::vec3 jsonVec3(jsonxx::Array const& values)
{
    if (values.size() != 3 || !values.has<jsonxx::Number>(0) || !values.has<jsonxx::Number>(1) || !values.has<jsonxx::Number>(2))
    {
        glow::error() << "Expected 3 numeric values for 3D vector, got " << values.json();
        return {};
    }

    return {values.get<jsonxx::Number>(0), values.get<jsonxx::Number>(1), values.get<jsonxx::Number>(2)};
}
static glm::mat3 jsonMat3(jsonxx::Array const& values)
{
    if (values.size() != 3)
    {
        glow::error() << "Expected 3 vectors for 3x3 matrix, got " << values.json();
        return {};
    }

    return transpose(glm::mat3(jsonVec3(values.get<jsonxx::Array>(0)), jsonVec3(values.get<jsonxx::Array>(1)),
                               jsonVec3(values.get<jsonxx::Array>(2))));
}
static glm::vec4 jsonVec4(jsonxx::Array const& values)
{
    if (values.size() != 4 || !values.has<jsonxx::Number>(0) || !values.has<jsonxx::Number>(1)
        || !values.has<jsonxx::Number>(2) || !values.has<jsonxx::Number>(3))
    {
        glow::error() << "Expected 4 numeric values for 4D vector, got " << values.json();
        return {};
    }

    return {values.get<jsonxx::Number>(0), values.get<jsonxx::Number>(1), values.get<jsonxx::Number>(2),
            values.get<jsonxx::Number>(3)};
}

static std::map<std::string, float> jsonProps(jsonxx::Object const& obj)
{
    if (!obj.has<jsonxx::Object>("properties"))
        return {};

    std::map<std::string, float> props;

    for (auto const& kvp : obj.get<jsonxx::Object>("properties").kv_map())
    {
        if (!kvp.second->is<jsonxx::Number>())
            glow::error() << "Custom property " << kvp.first << " is not a number";
        else
            props[kvp.first] = kvp.second->get<jsonxx::Number>();
    }

    return props;
}

static glm::vec3 aiCast(aiVector3D const& v)
{
    return {v.x, v.y, v.z};
}

void Level::loadFromFile(const std::string& blendFileName)
{
    ACTION(); // measure time used by this function
    using namespace jsonxx;

    glow::info() << "Loading level '" + blendFileName + "' ...";
    glow::timing::SystemTimer timer;

    // blend file does not exist
    if (!std::ifstream(blendFileName).good())
    {
        glow::error() << "Level file " << blendFileName << " does not exist!";
        exit(EXIT_FAILURE);
    }

    // check if exporter has to be run
    bool reExport = false;
    // .. either json file does not exist
    if (!std::ifstream(blendFileName + ".json").good())
        reExport = true;
    // .. or is older than blend file
    else if (glow::util::fileModificationTime(blendFileName)
             > glow::util::fileModificationTime(blendFileName + ".json"))
        reExport = true;

    // re-export if necessary
    if (reExport)
    {
        ACTION("Re-export Level");
        std::vector<std::string> blenderLocs;
        // Default location for windows
        blenderLocs.push_back("C:\\Program Files\\Blender Foundation\\Blender\\blender.exe");
        blenderLocs.push_back("C:\\Program Files (x86)\\Blender Foundation\\Blender\\blender.exe");
        std::string blenderCmd = "blender";
        for (auto const& loc : blenderLocs)
            if (std::ifstream(loc).good())
                blenderCmd = loc;
        auto cmd = "\"" + blenderCmd + "\" -b " + blendFileName + " -P level/exporter.py";
        glow::info() << "Re-exporting the Level";
        glow::info() << cmd;
        auto status = system(cmd.c_str());
        if (status != EXIT_SUCCESS)
        {
            glow::error() << "Unable to re-export level " << blendFileName;
            exit(EXIT_FAILURE);
        }
    }

    Object lvl;
    { // load file
        ACTION("Load Level JSON");
        mFileName = blendFileName;
        std::ifstream levelJson(blendFileName + ".json");
        lvl.parse(levelJson);
    }

    // dirs
    std::string giDirectory = mFileName + ".data/gi";

    // load content
    {
        ACTION("Create Level");

        // conversion between blender coordinate system and our (same as OBJ conversion)
        glm::mat3 blenderM = {glm::vec3(1, 0, 0),  //
                              glm::vec3(0, 0, -1), //
                              glm::vec3(0, 1, 0)};

        auto const& objects = lvl.get<Array>("objects");
        auto const& lights = lvl.get<Array>("lights");
		auto const& materials = lvl.get<Array>("materials");
        auto const& globals = lvl.get<Object>("global");

        // globals
        {
            glow::info() << " .. " << globals.size() << " global parameters";

            mAmbientColor = jsonVec3(globals.get<Array>("ambient"));
            mCustomProperties = jsonProps(globals);
        }

        // materials
        {
            glow::info() << " .. " << materials.size() << " materials";

            for (auto const& val : materials.values())
            {
                if (!val->is<Object>())
                {
                    glow::error() << "Material is not a json object";
                    continue;
                }

                auto const& mat = val->get<Object>();

                // ================================ MATERIAL ================================
                auto m = std::make_shared<Material>(mat.get<String>("name"));
                m->setAlpha(mat.get<Number>("alpha", 1.0));
                m->setAmbient(mat.get<Number>("ambient", 1.0));
                m->setDiffuseColor(jsonVec3(mat.get<Array>("diffuse")));
                m->setSpecularColor(jsonVec3(mat.get<Array>("specular")));
                m->setSpecularHardness(mat.get<Number>("specularHardness", 32.0));
                m->setUseObjectColor(mat.get<Boolean>("useObjectColor", false));
                m->setCustomProperties(jsonProps(mat));

				// ================================ TEXTURES ================================
				String basePath = ".";
				if (mat.has<Object>("textures")) {

					// load texture diffuse
					if (mat.get<Object>("textures").has<Object>("diffuse")) {
						TextureMaterial tDiffuse(TEX_DIFFUSE);
						tDiffuse.setPath(mat.get<Object>("textures").get<Object>("diffuse").get<String>("path"));
						tDiffuse.setColorSpace(mat.get<Object>("textures").get<Object>("diffuse").get<String>("colorspace"));
						tDiffuse.setScale(jsonVec2(mat.get<Object>("textures").get<Object>("diffuse").get<Array>("scale")));
						tDiffuse.setOffset(jsonVec2(mat.get<Object>("textures").get<Object>("diffuse").get<Array>("offset")));
						String path = tDiffuse.getPath();
						if (!path.empty()) {
							String texturePath = basePath + path;
							texturePath = remove_char(texturePath, "//..");
							tDiffuse.setTextureValue(glow::Texture2D::createFromFile(texturePath));
						}
						//push texture to material
						m->setTextureDiffuse(tDiffuse);
					}

					//load texture normal
					if (mat.get<Object>("textures").has<Object>("normal")) {
						TextureMaterial tNormal(TEX_NORMAL);
						tNormal.setPath(mat.get<Object>("textures").get<Object>("normal").get<String>("path"));
						tNormal.setColorSpace(mat.get<Object>("textures").get<Object>("normal").get<String>("colorspace"));
						tNormal.setScale(jsonVec2(mat.get<Object>("textures").get<Object>("normal").get<Array>("scale")));
						tNormal.setOffset(jsonVec2(mat.get<Object>("textures").get<Object>("normal").get<Array>("offset")));
						String path = tNormal.getPath();
						if (!path.empty()) {
							String texturePath = basePath + path;
							texturePath = remove_char(texturePath, "//..");
							tNormal.setTextureValue(glow::Texture2D::createFromFile(texturePath));
						}
						//push texture to material
						m->setTextureNormal(tNormal);
					}
				}
				
                addMaterial(m);
            }
        }

        // lights
        {
            glow::info() << " .. " << lights.size() << " lights";

            for (auto const& val : lights.values())
            {
                if (!val->is<Object>())
                {
                    glow::error() << "Light is not a json object";
                    continue;
                }

                auto const& light = val->get<Object>();

                // ================================ LIGHTS ================================
                auto type = light.get<String>("type");
                auto name = light.get<String>("name");
                SharedLightBase l;
                if (type == "SUN")
                {
                    auto dl = std::make_shared<DirectionalLight>(name);
                    dl->setDirection(blenderM * jsonVec3(light.get<Array>("direction")));
                    l = dl;

                    mDirectionalLights.push_back(dl);
                }
                else if (type == "POINT")
                {
                    auto pl = std::make_shared<PointLight>(name);
                    pl->setPosition(blenderM * jsonVec3(light.get<Array>("position")));
                    l = pl;

                    mPointLights.push_back(pl);
                }
                else
                {
                    glow::error() << "Unrecognized type '" << type << "' for light '" << name
                                  << "' (check capitalization)";
                    continue;
                }
                l->setColor(jsonVec3(light.get<Array>("color")));
                l->setType(type);
                l->setCustomProperties(jsonProps(light));
            }
        }

        // objects
        {
            glow::info() << " .. " << objects.size() << " objects";

            for (auto const& val : objects.values())
            {
                if (!val->is<Object>())
                {
                    glow::error() << "GameObject is not a json object";
                    continue;
                }
                auto const& obj = val->get<Object>();

                // ================================ OBJECTS ================================
                auto o = std::make_shared<GameObject>(obj.get<String>("name"));
                o->setPosition(blenderM * jsonVec3(obj.get<Array>("position")));
                o->setFrame(blenderM * jsonMat3(obj.get<Array>("frame")) * transpose(blenderM));
                o->setColor(jsonVec4(obj.get<Array>("color")));
                o->setCustomProperties(jsonProps(obj));

                // materials
                auto mats = obj.get<Array>("materials");
                std::vector<SharedMaterial> omats;
                for (auto const& mval : mats.values())
                {
                    if (!mval->is<String>())
                    {
                        glow::error() << "GameObject '" + o->getName() + "' material reference is not a string.";
                        continue;
                    }

                    auto mname = mval->get<String>();
                    if (!mMaterials.count(mname))
                    {
                        glow::error() << "GameObject '" + o->getName() + "'s material '" + mname + "' does not exist.";
                        continue;
                    }

                    omats.push_back(mMaterials[mname]);
                }
                o->setMaterials(omats);

                bool hasPhysics = obj.has<Object>("physics");

                // Mesh
				std::vector<Mesh::Vertex> vertices;
				std::vector<uint32_t> indices;
				std::vector<short> index;
				std::vector<float> _vert;
                {
                    auto meshFile = glow::util::pathOf(blendFileName) + "/" + obj.get<String>("filepath");
                    glow::info() << "   .. loading " << meshFile;
                    if (!std::ifstream(meshFile).good())
                    {
                        glow::error() << "Error loading `" << meshFile << "' with Assimp.";
                        glow::error() << "  File not found/not readable";
                        continue;
                    }

                    // Import by Assimp
                    auto mesh = std::make_shared<Mesh>();
                    uint32_t flags = aiProcess_SortByPType;
                    flags |= aiProcess_Triangulate;
                    flags |= aiProcess_CalcTangentSpace;
                    flags |= aiProcess_GenSmoothNormals; // in case no normals exist
                    flags |= aiProcess_GenUVCoords;      // in case no uv coords exist (but a mapping)
                    flags |= aiProcess_PreTransformVertices;

                    Assimp::Importer aimporter;
                    auto scene = aimporter.ReadFile(meshFile, flags);

                    if (!scene)
                    {
                        glow::error() << "Error loading `" << meshFile << "' with Assimp.";
                        glow::error() << "  " << aimporter.GetErrorString();
                        glow::error() << "GameObject '" + o->getName() + "'s mesh '" + meshFile
                                             + "' could not be loaded.";
                        continue;
                    }

                    if (!scene->HasMeshes())
                    {
                        glow::error() << "File `" << meshFile << "' has no meshes.";
                        glow::error() << "GameObject '" + o->getName() + "'s mesh '" + meshFile
                                             + "' could not be loaded.";
                        continue;
                    }

                    

                    auto baseIdx = 0u;
                    for (auto i = 0u; i < scene->mNumMeshes; ++i)
                    {
                        auto const& mesh = scene->mMeshes[i];
                        gdassert(mesh->HasNormals(), "mesh without normals is not supported");

                        // add faces
                        auto fCnt = mesh->mNumFaces;
                        for (auto f = 0u; f < fCnt; ++f)
                        {
                            auto const& face = mesh->mFaces[f];
                            gdassert(face.mNumIndices == 3, "non-triangle face");

                            for (auto fi = 0u; fi < face.mNumIndices; ++fi)
                                indices.push_back(baseIdx + face.mIndices[fi]);
                        }

                        // add vertices
                        auto vCnt = mesh->mNumVertices;
                        for (auto vi = 0u; vi < vCnt; ++vi)
                        {
                            Mesh::Vertex v;
                            v.position = aiCast(mesh->mVertices[vi]);
                            v.normal = aiCast(mesh->mNormals[vi]);
                            if (mesh->HasTangentsAndBitangents())
                                v.tangent = aiCast(mesh->mTangents[vi]);
                            if (mesh->HasTextureCoords(0))
                                v.texCoord = glm::vec2(aiCast(mesh->mTextureCoords[0][vi]));
                            v.lightCoord = glm::vec2(0, 0); // no coords in original obj
                            vertices.push_back(v);
                        }
                        baseIdx = vertices.size();
                    }

					for (auto i = 0u; i < vertices.size(); i++) {
						_vert.push_back(vertices[i].position.x);
						_vert.push_back(vertices[i].position.y);
						_vert.push_back(vertices[i].position.z);

					}

                    // unroll triangles
                    std::vector<Mesh::Triangle> triangles;
									
                    for (auto i = 0u; i < indices.size(); i += 3)
                    {
                        Mesh::Triangle t;
                        for (auto ti = 0; ti < 3; ++ti)
                            t.vertices[ti] = vertices[indices[i + ti]];
						
                        triangles.push_back(t);						
                    }

					index.resize(_vert.size());
					for(auto i = 0u; i<_vert.size(); ++i)
						index[i] = i;

                    mesh->setTriangles(triangles);
					

                    // apply scale part of rotation if this mesh is part of physics
                    // (bullet doesn't like scaling of rigid bodies)
                    if (hasPhysics)
                    {
                        auto newTransform = mesh->applyScale(o->getTransformation());
                        o->setTransformation(newTransform);
                    }

                    // set cpu mesh
                    o->setCpuMesh(mesh);

                    // create gpu mesh
                    o->setGpuMesh(mesh->uploadMesh());
                }


				
                // Physics
                if (hasPhysics)
                {
                    auto physics = obj.get<Object>("physics");

                    auto mass = physics.get<Number>("mass");
                    auto dynamic = physics.get<Boolean>("dynamic");
                    if (dynamic)
                    {
                        o->setUsesPrecomputedLighting(false);
                    }
                    else
                    {
                        mass = 0; // zero mass equals not simulated
                    }

                    // default motion state is object transformation
                    auto trans = o->getTransformation();					
                    // NOTE: this transformation has no scale component anymore!
                    auto motionState = new btDefaultMotionState(to_bullet(trans));


                    o->setMotionState(motionState);

                    // create shape
                    btCollisionShape* btshape = nullptr;
					btCollisionShape* btshape_c = nullptr;
					btBvhTriangleMeshShape* triMeshShape = nullptr;
                    btVector3 inertia(0, 0, 0);
                    auto shape = physics.get<String>("shape");

					///set the tag for each object
                    if (o->getName() == "Orange" || o->getName() == "Banana"||o->getName() == "Apple"||o->getName() == "Lemon")
						o->setTag("Fruit");
					else if (o->getName() == "Katana")
						o->setTag("Weapon");
					else if(o->getName() == "Bomb"){
						o->setTag("Bomb");
					}
					else {
						o->setTag("Static");
					}

                    ///set collision shape
					if (shape == "SPHERE")
                    {
						auto radius = o->getCpuMesh()->getCollisionSphereRadius();
                        btshape = new btSphereShape(radius);	
						btshape_c = new btSphereShape(radius);
                    }
                    else if (shape == "BOX")
                    {
                        auto halfExtends = o->getCpuMesh()->getCollisionBoxHalfExtends();
                        btshape = new btBoxShape(to_bullet(halfExtends));
						btshape_c = new btBoxShape(to_bullet(halfExtends));
                    }
                    else if (shape == "CYLINDER")
                    {
                        auto halfExtends = o->getCpuMesh()->getCollisionBoxHalfExtends();
                        btshape = new btCylinderShape(to_bullet(halfExtends));
						btshape_c = new btCylinderShape(to_bullet(halfExtends));
                    }
					else if (shape == "MESH")
					{
						
					}
					else if (shape == "CONVEX_HULL")
					{
						auto trimesh = createConvexTriangleMeshShape(o->getCpuMesh()->getTriangles());						
						btConvexShape *tmpshape = new btConvexTriangleMeshShape(trimesh);
						btConvexShape *tmpshape_c = new btConvexTriangleMeshShape(trimesh);
						btshape = tmpshape;
						btshape_c = tmpshape_c;
					}
                    else
                    {
                        glow::error() << "GameObject '" + o->getName() + "'s collision shape '" + shape
                                             + "' is not implemented/recognized.";
                        continue;
                    }

					o->setCollisionShape(btshape);

					if (btshape&&mass>0)
						btshape->calculateLocalInertia(mass, inertia);					
					
					// create rigid body
					if (mass>0) {
						btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, btshape, inertia);
						info.m_friction = physics.get<Number>("friction");
						info.m_restitution = physics.get<Number>("bounciness");
						info.m_linearDamping = physics.get<Number>("linearDamping");
						info.m_angularDamping = physics.get<Number>("angularDamping");
						auto rb = new btRigidBody(info);
						o->setRigidBody(rb);
					}
					else {
						btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, btshape);
						auto rb = new btRigidBody(info);
						o->setRigidBody(rb);
					}
					if (o->getName() == "Katana") {
						o->getRigidBody()->setCollisionFlags(o->getRigidBody()->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
						o->getRigidBody()->setActivationState(DISABLE_DEACTIVATION);
						btTransform t;
						o->getMotionState()->getWorldTransform(t);
						btMatrix3x3 orn = t.getBasis();
                        orn *= btMatrix3x3(btQuaternion(btVector3(1, 0, 0), -1.57));
                        orn *= btMatrix3x3(btQuaternion(btVector3(0, 1, 0), 1.57));
						t.setBasis(orn);
						swordState = t;
						o->getMotionState()->setWorldTransform(t);
					}	
					if (o->getTag() == "Fruit"|| o->getTag() == "Bomb") {
						fruitsForGenerate.push_back(fruitForGenerate(o, fruitNum++));
					}
                }

                if (o->getName() == "Banana") o->getRigidBody()->getWorldTransform().setOrigin(btVector3(5, 5, 5));
                if (o->getName() == "Apple") o->getRigidBody()->getWorldTransform().setOrigin(btVector3(5, 4, 5));
                // actually adds the object to the level
                addGameObject(o);
            }
        }
    }

    // re-compute globall illumination
    if (reExport)
    {
        // TODO: check for --no-gi-update flag
        computeGI(*this);
    }

    // load global illumination data
    {
        ACTION("Load Global Illumination Data");

        // light map texture
        const std::string giDirectory = mFileName + ".data/gi";
        mLightMap = glow::Texture2D::createFromFile(giDirectory + "/lightmap.png");
        if (!mLightMap)
        {
            // create blank light map texture as a fallback
            constexpr size_t textureSize = 32;
            std::vector<char> data(3 * textureSize * textureSize, 255);

            auto surfaceData = std::make_shared<glow::SurfaceData>();
            surfaceData->setWidth(textureSize);
            surfaceData->setHeight(textureSize);
            surfaceData->setFormat(GL_RGB);
            surfaceData->setType(GL_UNSIGNED_BYTE);
            surfaceData->setData(std::move(data));

            auto textureData = std::make_shared<glow::TextureData>();
            textureData->setWidth(textureSize);
            textureData->setHeight(textureSize);
            textureData->addSurface(surfaceData);

            mLightMap = glow::Texture2D::createFromData(textureData);
            mLightMap->bind().generateMipmaps();

            glow::warning() << "Could not load LightMap, use dummy one (black) instead";
        }

        // light map uv coords
        for (auto const& obj : mGameObjects)
        {
            if (obj->getUsesPrecomputedLighting())
            {
                const std::string uvFilename = giDirectory + "/" + obj->getName() + ".uv";
                if (std::ifstream{uvFilename}.good())
                {
                    std::ifstream file(uvFilename);

                    // store uv coords in GPU mesh
                    auto mesh = obj->getCpuMesh();
                    auto triangles = mesh->getTriangles();
                    for (auto& t : triangles)
                    {
                        for (int i = 0; i < 3; ++i)
                        {
                            float u, v;
                            file >> u >> v;
                            auto& vertex = t.vertices[i];
                            vertex.lightCoord = {u, v};
                        }

                        if (!file.good())
                        {
                            glow::error() << "Unexpected end of file while reading lightmap coords for GameObject '"
                                          << obj->getName() << "'";
                            break;
                        }
                    }
                    mesh->setTriangles(triangles);

                    // re-upload mesh
                    obj->setGpuMesh(mesh->uploadMesh());
                }
                else
                {
                    glow::error() << "Light map UV coords for GameObject '" + obj->getName()
                                         + "' not found. Expected location: " + uvFilename;
                    continue;
                }
            }
        }
    }

    glow::info() << "Loaded level '" + blendFileName + "' in " << (int)(timer.getTimeDiffInSecondsD() * 1000) << " ms.";
}
