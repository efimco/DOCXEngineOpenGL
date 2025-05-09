#include <iostream>
#include "stb_image.h"
#include "glad/glad.h"
#include "cubemap.hpp"


Cubemap::Cubemap(Camera& camera, std::string pathToCubemap): camera(camera)
{
	// Load cubemap textures
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); 
	float *data = stbi_loadf(pathToCubemap.c_str(), &width, &height, &nrChannels, 0);
	if (data) 
	{
		std::cout << "Loading cubemap" << std::endl;
		glCreateTextures(GL_TEXTURE_2D, 1, &cubemapID);

		glTextureStorage2D(cubemapID, 1, GL_RGB16F, width, height);
		glTextureSubImage2D(cubemapID, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);

		glTextureParameteri(cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
		glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		stbi_image_free(data);
	} else 
	{
		std::cerr << "Failed to load cubemap texture: " << pathToCubemap << std::endl;
		std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
	}
	stbi_set_flip_vertically_on_load(false);


	glCreateVertexArrays(1, &skyboxVAO);
	glCreateBuffers(1, &skyboxVBO);
	glNamedBufferData(skyboxVBO, sizeof(cubemapVertices), cubemapVertices, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(skyboxVAO, 0, skyboxVBO, 0, 3 * sizeof(float));
	glEnableVertexArrayAttrib(skyboxVAO, 0);
	glVertexArrayAttribFormat(skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(skyboxVAO, 0, 0);


	std::string vSkyboxShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\cubemap.vert").string();
	std::string fSkyboxShader = std::filesystem::absolute("..\\..\\src\\shaders\\cubemap\\cubemap.frag").string();

	cubemapShader = Shader(vSkyboxShader, fSkyboxShader);
}

Cubemap::~Cubemap()
{
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteTextures(1, &cubemapID);
}

void Cubemap::draw(glm::mat4 projection)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Cubemap Pass");
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		glDepthMask(GL_FALSE);
		glm::mat4 skyView = glm::mat4(glm::mat3(camera.getViewMatrix()));  
		cubemapShader.use();
		cubemapShader.setMat4("projection", projection);
		cubemapShader.setMat4("view", skyView);
		glBindTextureUnit(0, cubemapID);  // Use DSA binding
		
		glBindVertexArray(skyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glPopDebugGroup();
	};

