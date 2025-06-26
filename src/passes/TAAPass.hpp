#include "shader.hpp"


class TAAPass
{
public:
	TAAPass();
	void createOrResize();

private:
	Shader * m_TAAShader;
	uint32_t m_history;
	uint32_t  m_current;
	uint32_t m_FBOCurrent;
	uint32_t m_FBOHistory;
}