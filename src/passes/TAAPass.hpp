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
	void setJitterValues(glm::vec2 currentJitter, glm::vec2 prevJitter);
	uint32_t getCurrentOutput();
	
	uint32_t history0;
	uint32_t history1;
	

private:
	uint32_t m_current;
	uint32_t m_velocity;
	uint32_t m_prevVelocity;
	uint32_t m_depth;
	uint32_t m_prevDepth;
	uint32_t m_frameNumber;
	bool m_historyValid;
	bool m_pingPong; // false = write to history0, true = write to history1
	glm::vec2 m_currentJitter;
	glm::vec2 m_prevJitter;
	AppConfig& m_appConfig;
	Shader* m_TAAShader;
};