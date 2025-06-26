#pragma once
#include "shader.hpp"
// #include <glad/gl.h>
#include <string>

struct AppConfig
{
	// Window
	int windowWidth;
	int windowHeight;
	int renderWidth;
	int renderHeight;

	// Rendering
	float clearColor[4] = { 0.133f, 0.192f, 0.265f, 1.0f };
	float gamma = 1.0f;
	float nearPlane = 0.01f;
	float farPlane = 10000.0f;
	float exposure = 0.1f;
	uint32_t polygonMode = 0x1B02; // #define GL_FILL 0x1B02
	bool isWireframe = false;
	bool isFramebufferSizeSet = false;

	// Scene
	bool showObjectPicking = false;
	bool showShadowMap = false;
	float irradianceMapIntensity = 1.0f;
	float irradianceMapRotationY = 0.0f;
	std::string cubeMapPath;
	bool reloadCubeMap = false;

	// Post-processing
	float backgroundBlur = 0.0f;
	bool isTAA = false;

	// Shaders
	Shader* deferedShader = nullptr;

	// Singleton accessor
	static AppConfig& get();
	void initShaders();
};