#include <cstdint>
#include "pickingBuffer.hpp"
#include "depthBuffer.hpp"
class Renderer
{
public:
	Renderer();
	~Renderer();

	void initScreenQuad();
	void initDebugQuad();
	void initMainFrameBufferAndRenderTarget();
	void checkFrameBufeerSize();
	void render();
private:
uint32_t m_screenQuadVAO, m_screenQuadVBO;
uint32_t m_debugQuadVAO, m_debugQuadVBO;
uint32_t m_mainFbo, m_mainRbo, m_intermediateFBO, m_screenTexture;
};