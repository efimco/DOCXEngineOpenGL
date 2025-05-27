#include <cstdint>
#include "shader.hpp"

class OutlinePass
{
public:
	OutlinePass();
	void createOrResize();
	void draw(glm::mat4 projection, glm::mat4 view, uint32_t depthFBO);
	uint32_t silhouetteTexture;
	Shader* silhouetteShader;

private:
	uint32_t m_fbo;
	uint32_t m_rbo;
};