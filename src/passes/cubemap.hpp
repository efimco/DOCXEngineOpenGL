#pragma once
#include "camera.hpp"
#include "shader.hpp"
#include <cstdint>
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "appConfig.hpp"


class Cubemap
{
public:
	Shader backgroundShader;
	Shader equirectangularToCubemapShader;
	Shader irradianceShader;
	Shader specularShader;
	Shader brdfLutShader;
	uint32_t irradianceMap, specularMap, brdfLUTTexture, envCubemap;
	Camera& camera;
	Cubemap(Camera& camera, std::string pathToCubemap);
	~Cubemap();

	void createOrResize();
	void draw(glm::mat4 projection);

private:
	AppConfig& m_appConfig;
	uint32_t m_cubeVBO, m_cubeVAO;
	uint32_t m_quadVBO, m_quadVAO;
	uint32_t m_hdri;
	uint32_t m_captureFBO, m_captureRBO;
	uint32_t m_irradianceFBO, m_irradianceRBO;
	uint32_t m_specularFBO, m_specularRBO;
	uint32_t m_cubemapFBO;
	uint32_t m_envCubemap;
	void initCaptureFBO();
	void renderCube();
	void renderQuad();
	void convoluteIrradianceCubemap();
	void convoluteSpecularCubemap();
	void renderBRDFLUTTexture();
	void renderEquirectToCubemap();
	void loadHDR(std::string pathToCubemap);
	void initShaders();
	void createCubemap();
	void createIrradianceMap();
	void createSpecularMap();
	void createBRDFLUTTexture();

	const float m_quadCoords[8] = {
		// texture Coords
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f };
	const float m_quadPositions[12] = {
		// positions
		-1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f };

	const float m_cubemapVertices[108] = {
		// positions
		-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
		1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

		-1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
		-1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

		1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
		1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f };
	const glm::mat4 m_captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	const glm::mat4 m_captureViews[6] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)) };
};
