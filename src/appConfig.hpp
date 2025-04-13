#pragma once
#include "shader.hpp"
#include "glad/glad.h"



namespace AppConfig
{
	extern int WINDOW_WIDTH;
	extern int WINDOW_HEIGHT;
	extern float clearColor[4];
	extern Shader baseShader;
	extern Shader screenShader;
	extern Shader skyboxShader;
	extern Shader pickingShader;
	extern Shader depthShader;
	extern float gamma;
	extern float near_plane, far_plane;
	extern GLenum polygonMode;
	extern bool isWireframe;
	extern bool showObjectPicking;
	extern bool showShadowMap;
	extern bool isFramebufferSizeSetted;

	void initShaders();

}