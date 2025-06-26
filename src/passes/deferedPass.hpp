#pragma once
#include "shader.hpp"
#include "appConfig.hpp"
#include "camera.hpp"
#include "gBuffer.hpp"
#include "cubemap.hpp"
#include "pickingPass.hpp"
#include "shadowMap.hpp"
class DeferedPass
{
public:
	DeferedPass(Camera& camera);
	void createOrResize(uint32_t m_nMipLevels);
	void draw(uint32_t fullFrameQuadVAO, GBuffer* gBuffer, Cubemap* cubemap, ShadowMap* shadowMap, PickingPass* pickingPass);
	uint32_t deferedTexture;


private:
	Camera& m_camera;
	Shader* pickingShader;
	uint32_t m_deferedFBO, m_deferedRBO;
	AppConfig& m_appConfig;
};