#pragma once
#include <vector>
#include <array>
#include <glow/common/property.hh>
#include <glow/common/shared.hh>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glow/fwd.hh>
/**
* @brief The Mesh class represents a triangle mesh on the CPU
*/
class Mesh
{
public: // public structs
		/// vertex data
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent = { 0, 0, 0 };
		glm::vec2 texCoord = { 0, 0 };
		glm::vec2 lightCoord = { 0, 0 };
	};
	/// triangle data
	struct Triangle
	{
		std::array<Vertex, 3> vertices;
	};
private: // members
		 /// List of triangles
	std::vector<Triangle> mTriangles;
public: // getter, setter
	GLOW_PROPERTY(Triangles);
private:
	//Calculates the normal of the plane.
	glm::vec3 findNormal(std::vector<glm::vec3> plane);
	//Calculates the displacement of the plane (the 'd' in ax + by + cz + d = 0).
	float calculateDisplacement(glm::vec3 normal, std::vector<glm::vec3> plane);
	//Returns whether or not the given point is in front or behind the plane.
	float isInFront(glm::vec3 vertex, glm::vec3 norm, float d);
	/*
	Given the two sets of vertices calculates the vectors from from set to the other.
	Calculates the lines from these vectors and given vertices.
	Calculates the intersection of those lines with the plane and finally
	connects the two intersections to form the cut line.
	*/
	std::vector<Mesh::Vertex> calculateIntersection(std::vector<Mesh::Vertex> v1, std::vector<Mesh::Vertex> v2, std::vector<glm::vec3> plane, glm::vec3 normal);
	/*
	Given some percentage. linearly interpolate along the edge in order to
	calculate the new vertex values.
	Add those values to the lists and finally give the vertex the corresponding index's
	in order to find said values.
	*/
	Mesh::Vertex findVertex(float percentage, Mesh::Vertex from, Mesh::Vertex to);
	//Helper method for determining how far along the given line the intersection is.
	float getLineDisplacement(glm::vec3 position, glm::vec3 normal, glm::vec3 direction, std::vector<glm::vec3> cuttingPlane);
	//Given a set of vertices that make up a triangle and a quad. Cut the triangle that is made up of these shapes.
	std::vector<Mesh::Triangle> cutTriangle(std::vector < Mesh::Vertex > frontVertices, std::vector<Mesh::Vertex> backVertices, std::vector<glm::vec3> plane, glm::vec3 normal);
	//Helper method for finding the centroid of a triangle.
	glm::vec3 getCentroid(Mesh::Triangle t);
	//Converts the given quad into two triangles.
	std::vector<Mesh::Triangle> quadToTriangle(std::vector<Mesh::Vertex> quad);
	//Separates a single triangle making up the mesh of the geometry triangle translated by normal * direction. Quad triangles translated by normal * -direction.
	std::vector<Mesh::Triangle> separateTriangles(std::vector<Mesh::Triangle> triangles, int direction, glm::vec3 normal);
	Mesh::Triangle separateTriangle(Mesh::Triangle t, int direction, glm::vec3 normal);
	void addToTriangles(Mesh::Triangle triangle);
	//Helper method for getting the centroid vertex of a convex polygon.
	Mesh::Vertex getCentre(std::vector<Mesh::Vertex> polygon);
	glm::vec3 getGeometryCentre(std::vector<glm::vec3> points);
	//Creates a vector of triangles representing the mesh formed from the cut area.
	std::vector<Mesh::Triangle> getMesh(std::vector<Mesh::Vertex> vertices, Mesh::Vertex centre);
	//Helper method to get all the vertex points of Mesh
	std::vector<glm::vec3> getPoints();
	// check if a point lies inside the triangle
	bool inside_triangle(Mesh::Vertex a, Mesh::Vertex b, Mesh::Vertex c, Mesh::Vertex p);
	float triangle_area(Mesh::Vertex a, Mesh::Vertex b, Mesh::Vertex c);
public: // public functions
	Mesh();
	/// Gets a radius for a bullet collision sphere (centered on origin)
	float getCollisionSphereRadius() const;
	/// Gets half-extends for a bullet collision box (centered on origin)
	glm::vec3 getCollisionBoxHalfExtends() const;
	/// Creates a GPU version of this mesh
	glow::SharedVertexArray uploadMesh() const;
	/*
	For the given geometry and for every triangle within that geometry, if it intersects with the plane; cut the triangle and
	add the resulting triangles to either one of two geometrys (left of plane or right of plane).
	Return those geometrys.
	*/
	std::vector<Mesh> cutMesh(Mesh mesh, std::vector<glm::vec3> plane, bool samurai);
	/// Applies the scale part of the given rotation
	/// Returns a new transformation matrix with only rotation and translation
	glm::mat4 applyScale(glm::mat4 const& transform);
	/*
	For every geometry currently in the world, cut that geometry if it intersects with the plane
	and return new geometry resulting from the cut.
	*/
	std::vector<Mesh> createCut(std::vector<glm::vec3> plane, std::vector<Mesh> Meshes, bool g_samurai);
};