#include <glad/glad.h>
#include <cstdint>
#include <vector>
#include "shader.h"
#include <glm/glm.hpp>

class SceneObject
{
	public:
		uint32_t VBO;
		uint32_t VAO;
		std::vector<float> data;
		Shader shader;
		glm::mat4 view = 		glm::mat4(1.0f);
		glm::mat4 projection = 	glm::mat4(1.0f);
		glm::mat4 model =  		glm::mat4(1.0f);
		uint32_t texture;
		SceneObject(std::vector<float> data, Shader shader) : data(data), shader(shader){};

		void init()
		{
			glGenVertexArrays(1,&VAO);
			glGenBuffers(1, &VBO);
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, data.size() * 4, data.data(),GL_STATIC_DRAW);
			glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8 * sizeof(float),nullptr);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8 * sizeof(float),(void*)(5 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}

		void passTexture(uint32_t texture)
		{
			this -> texture = texture;
		}

		void draw(glm::mat4 projection, glm::mat4 view)
		{		
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(VAO);
			shader.use();
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);
			shader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(data.size() / 8));
		}

		void clean()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
		}
};