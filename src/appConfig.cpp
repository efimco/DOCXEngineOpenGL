#include "shader.hpp"
#include "glad/glad.h"
#include <string>
#include <filesystem>
#include "appConfig.hpp"

std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\multipleLightsSurface.frag").string();
std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\vertex.vert").string();

std::string fScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.frag").string();
std::string vScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.vert").string();

namespace AppConfig
{
	int WINDOW_WIDTH = 1024;
	int WINDOW_HEIGHT = 1024;
	int RENDER_WIDTH = 0;
	int RENDER_HEIGHT = 0;
	float clearColor[4] = { 0.133f, 0.192f, 0.265f, 1.0f };
	Shader baseShader{};
	Shader screenShader{};
	Shader debugDrawShader{};
	float gamma = 1;
	float near_plane = -2.01f;
	float far_plane = 2.0f;
	GLenum polygonMode = GL_FILL;
	bool isWireframe = false;
	bool showObjectPicking = false;
	bool showShadowMap = false;
	bool isFramebufferSizeSetted = true;
	float exposure = 0.1;
	float irradianceMapIntensity = 1;
	float irradianceMapRotationY = 0;
	std::string cubeMapPath = "";
	bool reloadCubeMap = false;

	void initShaders()
	{
		baseShader = Shader(vShaderPath, fShaderPath);
		screenShader = Shader(vScreenShader, fScreenShader);
	}

}