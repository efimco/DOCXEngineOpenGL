#include <glad/glad.h>
#include <cstdint>
#include <vector>
#include "shader.h"
#include <glm/glm.hpp>


enum TEXTURE_TYPE
{
	DIFFUSE,
	SPECULAR,
	NORMAL
};
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
		uint32_t tDiffuse;
		uint32_t tSpecular;
		uint32_t tNormal;
		SceneObject(std::vector<float> data, Shader shader) : data(data), shader(shader){};

		void init()
		{
			glGenVertexArrays(1,&VAO);
			glGenBuffers(1, &VBO);
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

		void passTexture(TEXTURE_TYPE type, uint32_t texture)
		{
			switch (type)
			{
			case DIFFUSE:
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture);
				shader.setInt("tDiffuse", 0);
				break;
			
			case SPECULAR:

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, texture);
				shader.setInt("tSpecular", 1);
				break;
			case NORMAL:
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, texture);
				shader.setInt("tNormal", 2);
				break;
			}
		}

		void draw(glm::mat4 projection, glm::mat4 view)
		{
			
			glBindVertexArray(VAO);
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