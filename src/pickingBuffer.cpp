#include <vector>
#include <glad/glad.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "primitive.hpp"
#include "sceneManager.hpp"
#include "pickingBuffer.hpp"
#include "appConfig.hpp"
#include <filesystem>




PickingBuffer::PickingBuffer()
	{
		init();
	}


void PickingBuffer::init()
{
	glCreateFramebuffers(1, &m_pickingFBO);
	glCreateRenderbuffers(1, &m_pickingRBO);

	glNamedRenderbufferStorage(m_pickingRBO, GL_DEPTH24_STENCIL8, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glNamedFramebufferRenderbuffer(m_pickingFBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_pickingRBO);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_pickingTexture);
	glTextureStorage2D(m_pickingTexture, 1, GL_RGB8, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glTextureParameteri(m_pickingTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(m_pickingTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(m_pickingTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_pickingTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glNamedFramebufferTexture(m_pickingFBO, GL_COLOR_ATTACHMENT0, m_pickingTexture, 0);


	int32_t width, height;
	glGetTextureLevelParameteriv(m_pickingTexture, 0, GL_TEXTURE_WIDTH, &width);
	glGetTextureLevelParameteriv(m_pickingTexture, 0, GL_TEXTURE_HEIGHT, &height);
	std::cout << "Picking texture size: " << width << "x" << height << std::endl;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	if (glCheckNamedFramebufferStatus(m_pickingFBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error: Picking FBO is not complete!" << std::endl;
	}

	const std::string vPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.vert").string();
	const std::string fPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.frag").string();

	pickingShader = Shader(vPickingShader, fPickingShader);
}

PickingBuffer::~PickingBuffer()
{
	glDeleteTextures(1, &m_pickingTexture);
	glDeleteFramebuffers(1, &m_pickingFBO);
	glDeleteRenderbuffers(1, &m_pickingRBO);
}

void PickingBuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glViewport(0, 0, AppConfig::RENDER_WIDTH, AppConfig::RENDER_HEIGHT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void PickingBuffer::draw(Camera& camera)
{

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Picking Pass");
		SceneManager::setShader(pickingShader);

		for (auto& primitive : SceneManager::getPrimitives())
		{
			glm::mat4 projection = glm::mat4(1.0f);
			if( AppConfig::RENDER_WIDTH != 0 && AppConfig::RENDER_HEIGHT != 0) 
			{
				projection = glm::perspective(glm::radians(camera.zoom), float(AppConfig::RENDER_WIDTH)/float(AppConfig::RENDER_HEIGHT),0.1f, 100.0f);	
			}
			primitive.shader.use();
			primitive.shader.setVec3("objectIDColor", setPickColor(primitive.vao));
		
			primitive.shader.setMat4("projection",projection);
			primitive.shader.setMat4("view",camera.getViewMatrix());
			primitive.shader.setMat4("model",primitive.transform);
			
			glBindVertexArray(primitive.vao);
			int eboSize = 0;
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
			int indexSize = eboSize / sizeof(int);

			glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (void*)0);
			glActiveTexture(GL_TEXTURE0);
			if (primitive.selected == true)
			{
				std::cerr << "drawing outline of: "  << primitive.vao << std::endl;
			}
			glBindVertexArray(0);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopDebugGroup();
}

void PickingBuffer::resize()
{
	if (m_pickingFBO == 0)
	{
		glDeleteTextures(1, &m_pickingTexture); 
		glDeleteRenderbuffers(1, &m_pickingRBO);
		glDeleteFramebuffers(1, &m_pickingFBO);
	}
	init();

	PickingBuffer();

}

glm::vec3 PickingBuffer::pickColorAt(double mouseX, double mouseY)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, m_pickingFBO);
	
	GLint texWidth = 0, texHeight = 0;
	glGetTextureParameteriv(m_pickingTexture, GL_TEXTURE_WIDTH, &texWidth);
	glGetTextureParameteriv(m_pickingTexture, GL_TEXTURE_HEIGHT, &texHeight);
	
	std::cout << "Reading from picking texture: " << texWidth << "x" << texHeight << std::endl;

	int readX = static_cast<int>(mouseX);
	int readY = static_cast<int>(abs(AppConfig::RENDER_HEIGHT-mouseY));


	GLubyte  pixel[4];
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(readX, readY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	// Add error checking
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "Error reading pixels: " << err << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glm::vec3 color = glm::vec3( pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2]/ 255.0f);

	std::cout << "Pixel: " << color.r << " " << color.g << " " << color.b << std::endl;
	return color;

}

Primitive* PickingBuffer::getIdFromPickColor(const glm::vec3 &color) 
{
	const float golden_ratio_conjugate = 0.618033988749895f;
	glm::vec3 hsv = rgb2hsv(color);
	float h = hsv.x;
	Primitive* closestObject  = nullptr;
	const unsigned int MAX_PICKABLE_OBJECTS = 1000;  // adjust as needed
	for (Primitive& primitive: SceneManager::getPrimitives()) 
		{
			float computedHue = glm::fract(primitive.vao * golden_ratio_conjugate);
			// Allow a small tolerance since floating-point imprecision can occur
			if (glm::abs(computedHue - h) < 0.01f) 
			{
				closestObject = &primitive;
				break;
			}
		}
	return closestObject;
}

uint32_t PickingBuffer::getPickingTexture() const
{
	return m_pickingTexture;
}

glm::vec3 PickingBuffer::setPickColor(unsigned int id)
{
	float golden_ratio_conjugate = 0.618033988749895f;
	float h = glm::fract(id * golden_ratio_conjugate);
	float s = 0.7f; // moderately saturated
	float v = 1.0f; // bright

	glm::vec3 idColor = hsv2rgb(h,s,v);
	return idColor;
}


glm::vec3 rgb2hsv(const glm::vec3 &rgb) 
{
	float r = rgb.r, g = rgb.g, b = rgb.b;
	float cmax = glm::max(r, glm::max(g, b));
	float cmin = glm::min(r, glm::min(g, b));
	float delta = cmax - cmin;
	float h = 0.0f;
	
	if (delta < 1e-6f) {
		h = 0.0f;
	}
	else if (cmax == r)
	{
		h = fmod((g - b) / delta, 6.0f);
	}
	else if (cmax == g)
	{
		h = ((b - r) / delta) + 2.0f;
	}
	else
	{
		h = ((r - g) / delta) + 4.0f;
	}
	h /= 6.0f;
	if (h < 0.0f)
		h += 1.0f;
	
	float s = (cmax <= 0.0f ? 0.0f : delta / cmax);
	float v = cmax;
	return glm::vec3(h, s, v);
}

glm::vec3 hsv2rgb(float h, float s, float v) {
	float c = v * s;
	float h_prime = h * 6.0f;
	float x = c * (1.0f - fabs(fmod(h_prime, 2.0f) - 1.0f));
	glm::vec3 rgb;
	
	if (h_prime < 1.0f)
		rgb = glm::vec3(c, x, 0.0f);
	else if (h_prime < 2.0f)
		rgb = glm::vec3(x, c, 0.0f);
	else if (h_prime < 3.0f)
		rgb = glm::vec3(0.0f, c, x);
	else if (h_prime < 4.0f)
		rgb = glm::vec3(0.0f, x, c);
	else if (h_prime < 5.0f)
		rgb = glm::vec3(x, 0.0f, c);
	else
		rgb = glm::vec3(c, 0.0f, x);
	
	float m = v - c;
	glm::vec3 res = rgb + glm::vec3(m);
	return res;
}


