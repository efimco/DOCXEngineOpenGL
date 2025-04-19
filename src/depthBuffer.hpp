#include <glad/glad.h>
#include <cstdint>

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
};