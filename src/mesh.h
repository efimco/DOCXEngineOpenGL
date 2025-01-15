#include <glm/ext/vector_float3.hpp>
#include <string>
#include <vector>
#include "shader.h"


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 texCoords;
};

struct Texture
{
	uint32_t id;
	std::string type;
};


class Mesh 
{
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Texture> textures;

		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures)
		{
			this -> vertices = vertices;
			this -> indices = indices;
			this -> textures = textures;
			setupMesh();
		}

		void Draw(Shader &shader)
		{
			uint32_t diffuseNr = 1;
			uint32_t specularNr = 1;

		}

	private:
		uint32_t VAO, VBO, EBO;
		void setupMesh()
		{
			glGenBuffers(1,&VAO);
			glGenBuffers(1,&VBO);
			glGenBuffers(1,&EBO);

			glBindVertexArray(VAO);


			glBindBuffer(GL_ARRAY_BUFFER,VBO);
			glBufferData(GL_ARRAY_BUFFER,vertices.size() * sizeof(Vertex),&vertices,GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size() * sizeof(uint32_t),&indices,GL_STATIC_DRAW);

			//vPos
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
			//normal
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
			//texCoords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

			glBindVertexArray(0);
		}


};	