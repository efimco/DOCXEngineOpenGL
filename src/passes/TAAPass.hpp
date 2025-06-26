#include "shader.hpp"
#include "appConfig.hpp"

class TAAPass
{
public:
	TAAPass();
	void draw();
	void createOrResize();
	void setCurrrentTexture(uint32_t curentTexture);
	void setVelocityTexture(uint32_t velocityTexture);
	void setDepthTexture(uint32_t depthTexture);
	uint32_t history;
	uint32_t current;
	uint32_t output;


private:
	uint32_t m_velocity;
	uint32_t m_depth;
	uint32_t m_frameNumber;
	AppConfig& m_appConfig;
	Shader* m_TAAShader;
	uint32_t m_FBOCurrent;
	uint32_t m_FBOHistory;
};