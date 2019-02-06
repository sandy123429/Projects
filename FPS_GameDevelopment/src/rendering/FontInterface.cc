#include "FontInterface.hh"

#include <glm/vec2.hpp>

#include <glow/gl.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/Program.hh>
#include <glow-extras/geometry/Quad.hh>

FontInterface::FontInterface() {
}

glm::vec3 generateColor(Color fontColor) {
	switch (fontColor){
		case WHITE:
			return glm::vec3(1, 1, 1); break;
		case BLACK:
			return glm::vec3(0, 0, 0); break;
		case RED:
			return glm::vec3(1, 0, 0); break;
		case BLUE:
			return glm::vec3(0, 0, 1); break;
		case GREEN:
			return glm::vec3(0, 1, 0); break;
	}
	return glm::vec3(1, 1, 1);
}

void FontInterface::InitText2d(const std::string &texturePath) {

	// Initialize font shader
	mShaderFont = glow::Program::createFromFiles({ "font/font.vsh",
		"font/font.fsh" });

	// Initialize background shader
	mShaderBackground = glow::Program::createFromFiles({"2d/2d.vsh",
		"2d/2d.fsh"});

	// Initialize texture
	mFontTexture = glow::Texture2D::createFromFile(texturePath);
}

void FontInterface::PrintText2d(std::string text, int x, int y, int size, Color fontColor) {

	glm::vec3 fontColorVector = generateColor(fontColor);

	GLOW_SCOPED(disable, GL_DEPTH_TEST);

	unsigned int length = text.length();

	auto vb = glow::ArrayBuffer::create();
	auto uv = glow::ArrayBuffer::create();

	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;

	// font texture map size 512 x 512
	// font box size 32 x 32
	for (unsigned int i = 0; i < length; i++) {
		if (text[i] == ' ')
			continue;

		glm::vec2 vertex_up_left = glm::vec2(x + i*size, y + size);
		glm::vec2 vertex_up_right = glm::vec2(x + i*size + size, y + size);
		glm::vec2 vertex_down_right = glm::vec2(x + i*size + size, y);
		glm::vec2 vertex_down_left = glm::vec2(x + i*size, y);

		vertices.push_back(vertex_up_left);
		vertices.push_back(vertex_down_left);
		vertices.push_back(vertex_up_right);

		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);

		char character = text[i] - 33;

		float uv_x = ((character) % 16) / 16.0f;
		float uv_y = ((character) / 16) / 16.0f;

		glm::vec2 uv_up_left = glm::vec2(uv_x, uv_y);
		glm::vec2 uv_up_right = glm::vec2(uv_x + 1.0f / 16.0f, uv_y);
		glm::vec2 uv_down_right = glm::vec2(uv_x + 1.0f / 16.0f, (uv_y + 1.0f / 16.0f));
		glm::vec2 uv_down_left = glm::vec2(uv_x, (uv_y + 1.0f / 16.0f));
		UVs.push_back(uv_up_left);
		UVs.push_back(uv_down_left);
		UVs.push_back(uv_up_right);

		UVs.push_back(uv_down_right);
		UVs.push_back(uv_up_right);
		UVs.push_back(uv_down_left);
	}

	auto shader = mShaderFont->use();

	// bind array buffer
	vb->defineAttribute<glm::vec2>("vertexPosition_screenspace");
	vb->bind().setData(vertices);

	uv->defineAttribute<glm::vec2>("vertexUV");
	uv->bind().setData(UVs);

	shader.setUniform("fontColor", fontColorVector);
	shader.setTexture("uTextureColor", mFontTexture);

	GLOW_SCOPED(enable, GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glow::VertexArray::create({ vb, uv })->bind().draw();

	GLOW_SCOPED(disable, GL_BLEND);
	GLOW_SCOPED(enable, GL_DEPTH_TEST);
}

void FontInterface::DrawOverlay()
{
	GLOW_SCOPED(disable, GL_DEPTH_TEST);

	auto vbb = glow::ArrayBuffer::create();;

	std::vector<glm::vec2> vertices;

	// draw vertices
	glm::vec2 vertex_up_left = glm::vec2(-1, 1);
	glm::vec2 vertex_up_right = glm::vec2(1, 1);
	glm::vec2 vertex_down_left = glm::vec2(-1, -1);
	glm::vec2 vertex_down_right = glm::vec2(1, 1);

	// first triangle
	vertices.push_back(vertex_up_left);
	vertices.push_back(vertex_up_right);
	vertices.push_back(vertex_down_left);

	// second triangle
	vertices.push_back(vertex_up_right);
	vertices.push_back(vertex_down_right);
	vertices.push_back(vertex_down_left);

	auto shaderBackground = mShaderBackground->use();

	// bind array buffer
	vbb->defineAttribute<glm::vec2>("vertexPosition");
	vbb->bind().setData(vertices);

	//shader.setUniform("inColor", colorTest);

	GLOW_SCOPED(enable, GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glow::VertexArray::create({ vbb })->bind().draw();

	GLOW_SCOPED(disable, GL_BLEND);
	GLOW_SCOPED(enable, GL_DEPTH_TEST);
}
