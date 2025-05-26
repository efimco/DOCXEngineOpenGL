#pragma once
#include "shader.hpp"
#include "camera.hpp"
#include <cstdint>
#include <vector>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>

class Cubemap
{
public:
	Shader backgroundShader;
	Shader equirectangularToCubemapShader;
	Shader irradianceShader;
	Shader specularShader;
	Shader brdfLutShader;
	uint32_t irradianceMap, envCubemap, specularMap, brdfLUTTexture;
	Camera &camera;
	Cubemap(Camera &camera, std::string pathToCubemap);
	~Cubemap();

	void draw(glm::mat4 projection);

private:
	uint32_t m_cubeVBO, m_cubeVAO;
	uint32_t m_quadVBO, m_quadVAO;
	uint32_t m_hdri;
	uint32_t m_captureFBO, m_captureRBO;
	uint32_t m_irradianceFBO, m_irradianceRBO;
	uint32_t m_specularFBO, m_specularRBO;
	void initCaptureFBO();
	void renderCube();
	void renderQuad();
	void convoluteIrradianceCubemap();
	void convoluteSpecularCubemap();
	void renderBRDFLUTTExture();
	void renderEquirectToCubemap();
	void loadHDR(std::string pathToCubemap);
	void initShaders();
	void createCubemap();
	void createIrradianceMap();
	void createSpecularMap();
	void createBRDFLUTTexture();

	const float m_quadCoords[8] =
		{
			// texture Coords
			0.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 1.0f,
			1.0f, 0.0f};
	const float m_quadPositions[12] =
		{
			// positions
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 0.0f};

	const float m_cubemapVertices[108] =
		{
			// positions
			-1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,

			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f};
	const glm::mat4 m_captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::mat4 m_captureViews[6] =
		{
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
			glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};
};
