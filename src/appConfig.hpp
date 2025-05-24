#pragma once
#include "shader.hpp"


typedef unsigned int GLenum;
namespace AppConfig
{
	extern int WINDOW_WIDTH;
	extern int WINDOW_HEIGHT;
	extern int RENDER_WIDTH;
	extern int RENDER_HEIGHT;
	extern float irradianceMapIntensity;
	extern float irradianceMapRotationY;
	extern float backgroundBlur;
	extern float clearColor[4];
	extern Shader baseShader;
	extern Shader screenShader;
	extern float gamma;
	extern float near_plane, far_plane;
	extern GLenum polygonMode;
	extern bool isWireframe;
	extern bool showObjectPicking;
	extern bool showShadowMap;
	extern bool isFramebufferSizeSetted;
	extern float exposure;
	extern std::string cubeMapPath;
	extern bool reloadCubeMap;

	void initShaders();

}