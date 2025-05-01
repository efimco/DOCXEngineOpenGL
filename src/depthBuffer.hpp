#include <glad/glad.h>
#include <cstdint>
#include "camera.hpp"
#include "glm/glm.hpp"

class DepthBuffer
{
public:
	const int width;
	const int height;
	uint32_t depthMapFBO;
	uint32_t depthMap;
	DepthBuffer(const int width, const int height);
	~DepthBuffer();	

	void bind();
	void unbind();
	void draw(Camera& camera, glm::mat4 lightSpaceMatrix, int32_t width, int32_t height);
};