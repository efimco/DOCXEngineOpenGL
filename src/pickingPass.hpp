#include <cstdint>
#include "shader.hpp"
#include <glm/glm.hpp>
class PickingPass
{
public:
	PickingPass();
	~PickingPass();
	void draw(glm::mat4 projection, glm::mat4 view);
	void createOrResize();
	uint32_t pickingTexture;
	Shader* pickingShader;
private:

	uint32_t m_pickingFBO;
	uint32_t m_pickingRBO;
};