#include <iostream>
#include "stb_image.h"
#include "glad/glad.h"
#include "cubemap.hpp"


Cubemap::Cubemap::Cubemap()
	{	

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemapID);

		// Instead of glTextureStorage2D, use glTexImage2D for each face
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
		for(unsigned int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		}

		glTextureParameteri(cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Load cubemap textures
		int width, height, nrChannels;
		for (unsigned int i = 0; i < cubemapFaces.size(); i++) {
			unsigned char *data = stbi_load(cubemapFaces[i].c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				std::cout << "Loading cubemap face " << i << ": " << cubemapFaces[i] << std::endl;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			} else {
				std::cerr << "Failed to load cubemap texture: " << cubemapFaces[i] << std::endl;
				std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
			}
		}


		glCreateVertexArrays(1, &skyboxVAO);
		glCreateBuffers(1, &skyboxVBO);
		glNamedBufferData(skyboxVBO, sizeof(cubemapVertices), cubemapVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(skyboxVAO, 0, skyboxVBO, 0, 3 * sizeof(float));
		glEnableVertexArrayAttrib(skyboxVAO, 0);
		glVertexArrayAttribFormat(skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(skyboxVAO, 0, 0);
	}

	Cubemap::~Cubemap()
	{
		glDeleteVertexArrays(1, &skyboxVAO);
		glDeleteBuffers(1, &skyboxVBO);
		glDeleteTextures(1, &cubemapID);
	}

void Cubemap::draw(Shader shader, glm::mat4 projection, glm::mat4 view)
	{
		glDepthMask(GL_FALSE);
		shader.use();
		shader.setInt("skybox", 0);  // Set texture unit
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTextureUnit(0, cubemapID);  // Use DSA binding
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);
	};

