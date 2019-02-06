#include "Mesh.hh"
#include <glow/common/log.hh>
#include <common/assert.hh>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>
//std::vector<glm::vec3> cutPlane;
std::vector<glm::vec3> originalPlane;
//glm::vec3 normal;
//float planeD;
//bool samurai;
Mesh::Mesh()
{
}
/* PUBLIC Functions Block*/
float Mesh::getCollisionSphereRadius() const
{
	float dis = 0;
	for (auto const& t : mTriangles)
		for (auto const& v : t.vertices)
			dis = glm::max(dis, glm::length(v.position));
	return dis;
}
glm::vec3 Mesh::getCollisionBoxHalfExtends() const
{
	glm::vec3 halfExtends;
	for (auto const& t : mTriangles)
		for (auto const& v : t.vertices)
			halfExtends = glm::max(halfExtends, glm::abs(v.position));
	return halfExtends;
}
glow::SharedVertexArray Mesh::uploadMesh() const
{
	auto ab = glow::ArrayBuffer::create();
	ab->defineAttribute(&Vertex::position, "aPosition");
	ab->defineAttribute(&Vertex::normal, "aNormal");
	ab->defineAttribute(&Vertex::tangent, "aTangent");
	ab->defineAttribute(&Vertex::texCoord, "aTexCoord");
	ab->defineAttribute(&Vertex::lightCoord, "aLightCoord");
	ab->bind().setData(sizeof(Triangle) * mTriangles.size(), mTriangles.data());
	return glow::VertexArray::create(ab);
}
glm::mat4 Mesh::applyScale(const glm::mat4& transform)
{
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transform, scale, rotation, translation, skew, perspective);
	gdassert(distance(skew, glm::vec3(0, 0, 0)) < 0.01, "No skew in transformation allowed");
	gdassert(distance(perspective, glm::vec4(0, 0, 0, 1)) < 0.01, "No projection in transformation allowed");
	// apply scale
	glm::mat3 scaleT(glm::scale(scale));
	auto normalT = inverse(scaleT); // already transposed
	for (auto& t : mTriangles)
		for (auto& v : t.vertices)
		{
			v.position = scaleT * v.position;
			v.normal = normalize(normalT * v.normal);
			if (length(v.tangent) > .001) // might be zero
				v.tangent = normalize(normalT * v.tangent);
		}
	return transform * glm::scale(1.f / scale);
}
std::vector<Mesh> Mesh::createCut(std::vector<glm::vec3> plane, std::vector<Mesh> Meshes, bool g_samurai)
{
	//cutPlane = plane;
	originalPlane = plane;
	//samurai = g_samurai;
	int triCount = 0;
	std::vector<Mesh> allMesh;
	for (Mesh mesh : Meshes)
	{
		std::vector<Mesh> newMeshes;
		newMeshes = cutMesh(mesh, plane, g_samurai);
		for (Mesh newMesh : newMeshes)
		{
			allMesh.push_back(newMesh);
			triCount += newMesh.getTriangles().size();
		}
	}
	return allMesh;
}
/*PRIVATE Functions Block*/
bool Mesh::inside_triangle(Mesh::Vertex a, Mesh::Vertex b, Mesh::Vertex c, Mesh::Vertex p)
{
	float A = triangle_area(a, b, c);
	float A1 = triangle_area(p, a, b);
	float A2 = triangle_area(p, b, c);
	float A3 = triangle_area(p, a, c);
	if (A == A1 + A2 + A3)
		return true;
	else
		return false;
}
float Mesh::triangle_area(Mesh::Vertex a, Mesh::Vertex b, Mesh::Vertex c)
{
	glm::vec3 cr = cross(c.position - a.position, b.position - a.position);
	return (0.5*sqrt(cr.x*cr.x + cr.y*cr.y + cr.z*cr.z));
}
std::vector<Mesh> Mesh::cutMesh(Mesh mesh, std::vector<glm::vec3> plane, bool samurai)
{
	Mesh leftMesh = Mesh();
	Mesh rightMesh = Mesh();
	int intersects = 0;
	std::vector<Mesh::Triangle> tempTriangles;
	std::vector<Mesh::Vertex> cutVertices;
	glm::vec3 meshPosition = getGeometryCentre(mesh.getPoints());
	std::cout << "meshposition " << meshPosition.x << " " << meshPosition.y << " " << meshPosition.z << std::endl;
	//Translate plane back to vertex data
	for (int i = 0; i < 3; i++) {
		plane[i].x = plane[i].x - meshPosition.x;
		plane[i].y = plane[i].y - meshPosition.y;
		plane[i].z = plane[i].z - meshPosition.z;
	}
	glm::vec3 normal = findNormal(plane);
	float planeD = calculateDisplacement(normal, plane);
	for (Mesh::Triangle tri : mesh.getTriangles())
	{
		std::vector<Mesh::Vertex> intersectVertices;
		std::vector<Mesh::Vertex> frontVertices;
		std::vector<Mesh::Vertex> backVertices;
		//Separate the Vertices
		for (Mesh::Vertex v : tri.vertices)
		{
			glm::vec3 point = v.position;
			if (isInFront(point, normal, planeD) > 0.0f) {
				frontVertices.push_back(v);
			}
			else
				backVertices.push_back(v);
		}
		//Calculate the intersection.
		if (frontVertices.size() > 0 && backVertices.size() > 0) {	//If triangle intersects with the plane.
			intersects = 1;
			if (frontVertices.size() > backVertices.size()) {
				intersectVertices = calculateIntersection(backVertices, frontVertices, plane, normal);
			}
			else {
				intersectVertices = calculateIntersection(frontVertices, backVertices, plane, normal);
			}
			//Add the new vertices to the old ones
			for (Mesh::Vertex vertex : intersectVertices) {
				frontVertices.push_back(vertex);
				backVertices.push_back(vertex);
				cutVertices.push_back(vertex);
			}
			std::vector<Mesh::Triangle> newTriangles;
			//Actually cut it.
			newTriangles = cutTriangle(frontVertices, backVertices, plane, normal);
			glm::vec3 tri1Centroid = getCentroid(newTriangles[0]);	//Centroid of the triangle
			glm::vec3 tri2Centroid = getCentroid(newTriangles[1]);	//Centroid of one of the triangles within the quad
																	//Add the new triangles to the corresponding geometry.
			if (isInFront(tri1Centroid, normal, planeD) > 0.0f) {
				leftMesh.addToTriangles(newTriangles[0]);
			}
			else {
				rightMesh.addToTriangles(newTriangles[0]);
			}
			if (isInFront(tri2Centroid, normal, planeD) > 0.0f) {
				leftMesh.addToTriangles(newTriangles[1]);
				rightMesh.addToTriangles(newTriangles[2]);
			}
			else {
				leftMesh.addToTriangles(newTriangles[1]);
				rightMesh.addToTriangles(newTriangles[2]);
			}
		}
		else
			tempTriangles.push_back(tri);	//Store all the triangles within the geometry that haven't been cut so that we can
											//translate them later if the geometry was cut.	
	}
	//Translate the uncut triangles of the geometry
	for (Mesh::Triangle t : tempTriangles) {
		if (intersects == 1) {
			glm::vec3 centroid = getCentroid(t);
			float distance = dot(normal, (centroid - plane[0]));
			Mesh::Triangle newTriangle;
			if (distance > 0) {
				newTriangle = separateTriangle(t, 1, normal);
			}
			else {
				newTriangle = separateTriangle(t, -1, normal);
			}
			if (isInFront(centroid, normal, planeD) > 0.0f) {
				leftMesh.addToTriangles(newTriangle);
			}
			else {
				rightMesh.addToTriangles(newTriangle);
			}
		}
		else {
			if (isInFront(getCentroid(t), normal, planeD) > 0.0f) {
				leftMesh.addToTriangles(t);
			}
			else {
				rightMesh.addToTriangles(t);
			}
		}
	}
	std::cout << "cut vertices size: " << cutVertices.size() << std::endl;
	//If geometry was cut
	//TODO : ear clipping 
	if (cutVertices.size() > 0) {
		//Create a new mesh for the cut area
		Mesh::Vertex centroidPoly = getCentre(cutVertices);
		std::vector<Mesh::Triangle> mesh = getMesh(cutVertices, centroidPoly);
		std::vector<Mesh::Triangle> mesh2 = getMesh(cutVertices, centroidPoly);
		//Split the mesh into two
		for (Mesh::Triangle t : mesh) {
			Mesh::Triangle newTriangle;
			newTriangle = separateTriangle(t, 1, normal);
			leftMesh.addToTriangles(newTriangle);
		}
		for (Mesh::Triangle t : mesh2) {
			Mesh::Triangle newTriangle;
			newTriangle = separateTriangle(t, -1, normal);
			rightMesh.addToTriangles(newTriangle);
		}
	}
	//If plane didn't intersect this geometry then one of the geometry's will be empty. So discard it.
	std::vector<Mesh> bothGeometrys;
	if (leftMesh.getTriangles().size() == 0) {	//If this geometry is empty, add rightmesh
												//geometry2.setRigidbody(parent);
		std::wcout << "left mesh empty" << std::endl;
		bothGeometrys.push_back(rightMesh);
	}
	else if (rightMesh.getTriangles().size() == 0) {	//If this geometry is empty, add leftmesh
														//geometry1.setRigidbody(parent);
		std::cout << "right mesh emp" << std::endl;
		bothGeometrys.push_back(leftMesh);
	}
	else {	//Else add  both as this geometry has been cut
		glm::vec3 rigidBase;
		if (samurai) {
			rigidBase = getGeometryCentre(leftMesh.getPoints()) / 100;
		}
		else {
			rigidBase = getGeometryCentre(leftMesh.getPoints());
		}
		/*Rigidbody *child = new Rigidbody(rigidBase + parent->position, geometry1.getPoints(), 1, geometry1.getPoints().size(), parent->force);
		p->addRigidbody(child);
		geometry1.setRigidbody(child);
		*/
		bothGeometrys.push_back(leftMesh);
		if (samurai) {
			rigidBase = getGeometryCentre(rightMesh.getPoints()) / 100;
		}
		else {
			rigidBase = getGeometryCentre(rightMesh.getPoints());
		}
		/*cout << "Geometry2 Mesh position is: " << rigidBase << endl;
		Rigidbody *child2 = new Rigidbody(rigidBase + parent->position, geometry2.getPoints(), 1, geometry2.getPoints().size(), parent->force);
		p->addRigidbody(child2);
		geometry2.setRigidbody(child2);
		*/
		bothGeometrys.push_back(rightMesh);
		//p->remove(g_geometry.getRigidbody());
	}
	plane = originalPlane;
	return bothGeometrys;
}
glm::vec3 Mesh::findNormal(std::vector<glm::vec3> plane)
{
	glm::vec3 edge1 = plane[0] - plane[1];
	glm::vec3 edge2 = plane[2] - plane[1];
	glm::vec3 norm = glm::cross(edge1, edge2);
	return norm;
}
float Mesh::calculateDisplacement(glm::vec3 normal, std::vector<glm::vec3> plane)
{
	float d = (normal.x*plane[0].x*-1) - (normal.y*plane[0].y) - (normal.z*plane[0].z);
	return d;
}
float Mesh::isInFront(glm::vec3 vertex, glm::vec3 norm, float d)
{
	return ((norm.x*vertex.x) + (norm.y*vertex.y) + (norm.z*vertex.z) + d);
}
std::vector<Mesh::Vertex> Mesh::calculateIntersection(std::vector<Mesh::Vertex> v1, std::vector<Mesh::Vertex> v2, std::vector<glm::vec3> plane, glm::vec3 normal)
{
	//The two vectors
	glm::vec3 vector1 = v2[0].position - v1[0].position;
	glm::vec3 vector2 = v2[1].position - v1[0].position;
	//The magnitude of these vectors
	double vector1Magntde = pow((pow(vector1.x, 2) + pow(vector1.y, 2) + pow(vector1.z, 2)), 0.5);
	double vector2Magntde = pow((pow(vector2.x, 2) + pow(vector2.y, 2) + pow(vector2.z, 2)), 0.5);
	//The unit vectors
	glm::vec3 vector1Unit = vector1 * (1 / vector1Magntde);
	glm::vec3 vector2Unit = vector2 * (1 / vector2Magntde);
	//The scaler t for the intersection
	float t = getLineDisplacement(v1[0].position, normal, vector1Unit, plane);
	float t2 = getLineDisplacement(v1[0].position, normal, vector2Unit, plane);
	//The new vertices at the intersection point
	Mesh::Vertex intersectVertex1 = findVertex(t / vector1Magntde, v1[0], v2[0]);
	Mesh::Vertex intersectVertex2 = findVertex(t2 / vector2Magntde, v1[0], v2[1]);
	std::vector<Mesh::Vertex> vertices;
	vertices.push_back(intersectVertex1);
	vertices.push_back(intersectVertex2);
	return vertices;
}
float Mesh::getLineDisplacement(glm::vec3 position, glm::vec3 normal, glm::vec3 direction, std::vector<glm::vec3> plane) {
	float t = dot((plane[0] - position), normal) / (dot(direction, normal));
	return t;
}
Mesh::Vertex Mesh::findVertex(float percentage, Mesh::Vertex from, Mesh::Vertex to)
{
	glm::vec3 normal = ((to.normal - from.normal) * percentage) + from.normal;
	glm::vec2 uv = ((to.texCoord - from.texCoord) * percentage) + from.texCoord;
	glm::vec3 point = ((to.position - from.position) * percentage) + from.position;
	Mesh::Vertex newVertex;
	newVertex.normal = normal;
	newVertex.texCoord = uv;
	newVertex.position = point;
	return newVertex;
}
std::vector<Mesh::Triangle> Mesh::cutTriangle(std::vector < Mesh::Vertex > frontVertices, std::vector<Mesh::Vertex> backVertices, std::vector<glm::vec3> plane, glm::vec3 normal)
{
	std::vector<Mesh::Vertex> quad;
	Mesh::Triangle tri;
	if (frontVertices.size() > backVertices.size()) {
		quad = frontVertices;
		tri.vertices[0] = backVertices[0];
		tri.vertices[1] = backVertices[1];
		tri.vertices[2] = backVertices[2];
	}
	else {
		quad = backVertices;
		tri.vertices[0] = frontVertices[0];
		tri.vertices[1] = frontVertices[1];
		tri.vertices[2] = frontVertices[2];
	}
	glm::vec3 centroidTri = getCentroid(tri);
	//Convert quad to triangles
	std::vector<Mesh::Triangle> quadTriangles = quadToTriangle(quad);
	std::vector<Mesh::Triangle> triangles;
	triangles.push_back(tri);
	triangles.push_back(quadTriangles[0]);
	triangles.push_back(quadTriangles[1]);
	//The shortest distance between the triangle centroid and the cutPlane.
	float distance = dot(normal, (centroidTri - plane[0]));
	std::vector<Mesh::Triangle> newTriangles;
	//If centroidTri lies on same side as the normal.
	if (distance > 0) {
		//Draw the new triangles.
		newTriangles = separateTriangles(triangles, 1, normal);
	}
	else if (distance < 0) {
		newTriangles = separateTriangles(triangles, -1, normal);
	}
	else {
		//Draw the original (hasn't been cut yet).
		newTriangles = separateTriangles(triangles, 0, normal);
	}
	return newTriangles;
}
glm::vec3 Mesh::getCentroid(Mesh::Triangle t)
{
	GLfloat centerX = (t.vertices[0].position.x + t.vertices[1].position.x + t.vertices[2].position.x) / 3;
	GLfloat centerY = (t.vertices[0].position.y + t.vertices[1].position.y + t.vertices[2].position.y) / 3;
	GLfloat centerZ = (t.vertices[0].position.z + t.vertices[1].position.z + t.vertices[2].position.z) / 3;
	glm::vec3 centroid(centerX, centerY, centerZ);
	return centroid;
}
std::vector<Mesh::Triangle> Mesh::quadToTriangle(std::vector<Mesh::Vertex> quad)
{
	Mesh::Triangle triangle1;
	Mesh::Triangle triangle2;
	for (int i = 0; i < quad.size(); i++) {
		if (i == 0) {
			triangle1.vertices[0] = (quad[i]);
			triangle2.vertices[0] = (quad[i]);
		}
		else if (i == 1) {
			triangle2.vertices[1] = (quad[i]);
		}
		else if (i == 2) {
			triangle1.vertices[1] = (quad[i]);
		}
		else if (i == 3) {
			triangle1.vertices[2] = (quad[i]);
			triangle2.vertices[2] = (quad[i]);
		}
	}
	std::vector<Mesh::Triangle> triangles;
	triangles.push_back(triangle1);
	triangles.push_back(triangle2);
	return triangles;
}
std::vector<Mesh::Triangle> Mesh::separateTriangles(std::vector<Mesh::Triangle> triangles, int direction, glm::vec3 normal) {
	std::vector<Mesh::Triangle> newTriangles = triangles;
	glm::vec3 translateDirection = normal * direction;
	//Translation magnitude
	double normalMagntde = pow((pow(translateDirection.x, 2) + pow(translateDirection.y, 2) + pow(translateDirection.z, 2)), 0.5);
	//Translation direction
	glm::vec3 translateUnit = translateDirection * (1 / normalMagntde);
	translateUnit = translateUnit * 0;
	for (int i = 0; i < newTriangles.size(); i++) {
		for (Mesh::Vertex &v : newTriangles[i].vertices) {
			if (i == 0 && direction != 0) {
				v.position.x = v.position.x + translateUnit.x;
				v.position.y = v.position.y + translateUnit.y;
				v.position.z = v.position.z + translateUnit.z;
			}
			else if (direction != 0) {
				v.position.x = v.position.x + translateUnit.x * -1;
				v.position.y = v.position.y + translateUnit.y * -1;
				v.position.z = v.position.z + translateUnit.z * -1;
			}
		}
	}
	return newTriangles;
}
void Mesh::addToTriangles(Mesh::Triangle triangle) {
	mTriangles.push_back(triangle);
}
Mesh::Triangle Mesh::separateTriangle(Mesh::Triangle t, int direction, glm::vec3 normal) {
	Mesh::Triangle newTriangle = t;
	glm::vec3 translateDirection = normal * direction;
	//Translation magnitude
	double normalMagntde = pow((pow(translateDirection.x, 2) + pow(translateDirection.y, 2) + pow(translateDirection.z, 2)), 0.5);
	//Translation direction
	glm::vec3 translateUnit = translateDirection * (1 / normalMagntde);
	translateUnit = translateUnit * 0;
	for (Mesh::Vertex &v : newTriangle.vertices) {
		v.position.x = v.position.x + translateUnit.x;
		v.position.y = v.position.y + translateUnit.y;
		v.position.z = v.position.z + translateUnit.z;
	}
	return newTriangle;
}
Mesh::Vertex Mesh::getCentre(std::vector<Mesh::Vertex> polygon) {
	GLfloat centreNX = 0;
	GLfloat centreNY = 0;
	GLfloat centreNZ = 0;
	GLfloat centreTX = 0;
	GLfloat centreTY = 0;
	GLfloat centrePX = 0;
	GLfloat centrePY = 0;
	GLfloat centrePZ = 0;
	for (Mesh::Vertex v : polygon) {
		centreNX = centreNX + v.normal.x;
		centreNY = centreNY + v.normal.y;
		centreNZ = centreNZ + v.normal.z;
		centreTX = centreTX + v.texCoord.x;
		centreTY = centreTY + v.texCoord.y;
		centrePX = centrePX + v.position.x;
		centrePY = centrePY + v.position.y;
		centrePZ = centrePZ + v.position.z;
	}
	int size = polygon.size();
	glm::vec3 centreN(centreNX / size, centreNY / size, centreNZ / size);
	glm::vec2 centreT(centreTX / size, centreTY / size);
	glm::vec3 centreP(centrePX / size, centrePY / size, centrePZ / size);
	Mesh::Vertex centreVertex;
	centreVertex.normal = centreN;
	centreVertex.texCoord = centreT;
	centreVertex.position = centreP;
	return centreVertex;
}
std::vector<Mesh::Triangle> Mesh::getMesh(std::vector<Mesh::Vertex> vertices, Mesh::Vertex centre) {
	std::vector<Mesh::Triangle> mesh;
	int count = 0;
	for (int i = 0; i < vertices.size() - 1; i++) {
		if (count == 1) {
			count = 0;
			continue;
		}
		Mesh::Triangle t;
		t.vertices[0] = centre;
		t.vertices[1] = vertices[i];
		t.vertices[2] = vertices[i + 1];
		mesh.push_back(t);
		count++;
	}
	/*std::vector<Mesh::Vertex> initialPoints = vertices;
	if (vertices.size() < 3) // let's make sure that the user don't feed the function with less than 3 points !
	return mesh;
	else
	{
	bool impossibleToTriangulate = false;
	bool triangleFound = true;
	while (vertices.size() != 0) // run the algorithm until our polygon is empty
	{
	if (!triangleFound) // if we've looped once without finding any ear, the program is stuck, the polygon is not triangulable for our algorithm (likely to be a 8 shape or such self intersecting polygon)
	return mesh;
	triangleFound = false; // we want to find a new ear at each loop
	for (int i(0); i < vertices.size() - 2; i++) // for each 3 consecutive points we check if it's an ear : an ear is a triangle that wind in the right direction and that do not contain any other point of the polygon
	{
	if (!triangleFound) // if we still didn't find an ear
	{
	bool result = false;
	if (dot(vertices[i + 1].position - vertices[i].position, vertices[i + 2].position - vertices[i + 1].position) < 0) // if the triangle winds in the right direction
	{
	result = true;
	for (int j(0); j < initialPoints.size(); j++) // we check if there's no point inside it
	{
	if (inside_triangle(vertices[i + 2], vertices[i + 1], vertices[i], initialPoints[j]))
	{
	result = false; // if I got a point in my triangle, then it's not an ear !
	}
	}
	}
	if (result) // now, we have found an ear :
	{
	triangleFound = true;
	Mesh::Triangle t;
	t.vertices[0] = vertices[i];
	t.vertices[1] = vertices[i + 1];
	t.vertices[2] = vertices[i + 2];
	mesh.push_back(t);
	std::vector<Mesh::Vertex> bufferArray;
	for (int j(0); j < vertices.size(); j++) // then we delete the triangle in the points array : we already know that it's an ear, we don't need it anymore
	{
	if (j != i + 1) // we copiy all the points in a buffer array except the point we don't want
	{
	bufferArray.push_back(vertices[j]);
	}
	}
	vertices = bufferArray;
	}
	}
	}
	}
	}
	//return triangles; // we return the triangle array
	*/
	return mesh;
}
glm::vec3 Mesh::getGeometryCentre(std::vector<glm::vec3> points) {
	GLfloat centreX = 0;
	GLfloat centreY = 0;
	GLfloat centreZ = 0;
	for (glm::vec3 v : points) {
		centreX = centreX + v.x;
		centreY = centreY + v.y;
		centreZ = centreZ + v.z;
	}
	glm::vec3 centroid(centreX / points.size(), centreY / points.size(), centreZ / points.size());
	return centroid;
}
std::vector<glm::vec3> Mesh::getPoints() {
	std::vector<glm::vec3> m_points;
	for (Mesh::Triangle t : mTriangles) {
		m_points.push_back(t.vertices[0].position);
		m_points.push_back(t.vertices[1].position);
		m_points.push_back(t.vertices[2].position);
	}
	return m_points;
}
