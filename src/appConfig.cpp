#include "shader.hpp"
#include "glad/glad.h"
#include <string>
#include <filesystem>
#include "appConfig.hpp"

std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\multipleLightsSurface.frag").string();
std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\vertex.vert").string();

std::string fScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.frag").string();
std::string vScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.vert").string();

std::string vSkyboxShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\cubemap.vert").string();
std::string fSkyboxShader = std::filesystem::absolute("..\\..\\src\\shaders\\\\cubemap\\cubemap.frag").string();

std::string vPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.vert").string();
std::string fPickingShader = std::filesystem::absolute("..\\..\\src\\shaders\\picking.frag").string();

std::string simpleDepthShader = std::filesystem::absolute("..\\..\\src\\shaders\\simpleDepthShader.vert").string();
std::string fEmptyShader = std::filesystem::absolute("..\\..\\src\\shaders\\empty.frag").string();

namespace AppConfig
{
	int WINDOW_WIDTH = 1024;
	int WINDOW_HEIGHT = 1024;
	float clearColor[4] = { 0.133f, 0.192f, 0.265f, 1.0f };
	Shader baseShader{};
	Shader screenShader{};
	Shader skyboxShader{};
	Shader pickingShader{};
	Shader depthShader{};
	float gamma = 1;
	float near_plane = -2.01f;
	float far_plane = 2.0f;
	GLenum polygonMode = GL_FILL;
	bool isWireframe = false;
	bool showObjectPicking = false;
	bool showShadowMap = false;
	bool isFramebufferSizeSetted = true;
	float exposure = 0.1;

	void initShaders()
	{
		// Initialize shaders after OpenGL context is created
		baseShader = Shader(vShaderPath, fShaderPath);
		screenShader = Shader(vScreenShader, fScreenShader);
		skyboxShader = Shader(vSkyboxShader, fSkyboxShader);
		pickingShader = Shader(vPickingShader, fPickingShader);
		depthShader = Shader(simpleDepthShader, fEmptyShader);
	}

}