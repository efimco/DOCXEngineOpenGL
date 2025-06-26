#include "shader.hpp"
#include "appConfig.hpp"


class PostProcessPass
{
public:
	PostProcessPass();
	void createOrResize();
	uint32_t getPostProcessedTexture() const;
	void draw(uint32_t prePostProcessTexture, uint32_t cubemap, uint32_t depthTexture);
private:
	AppConfig& m_appConfig;
	Shader* m_postProcessShader;
	uint32_t m_postProcessedTexture;
};