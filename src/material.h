#pragma once
#include <string>
#include "stb_image.h"


struct Tex
{
	uint32_t id;
	std::string type;
	std::string path;
	void SetPath(const std::string& newPath) 
	{
		path = newPath;
		id = TextureFromFile(newPath.c_str());
	}

	Tex(const char* path, std::string type):path(path), type(type){};
	~Tex(){};

	uint32_t TextureFromFile(const char *path)
		{
			std::string filename = std::string(path);
			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
			if (data)
			{
				GLenum format;
				if (nrComponents == 1)
					format = GL_RED;
				else if (nrComponents == 3)
					format = GL_RGB;
				else if (nrComponents == 4)
					format = GL_RGBA;

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else
			{
				// std::cout << "Texture failed to load at path: " << path << std::endl;
				stbi_image_free(data);
			}

			return textureID;
		}
};

struct Mat
{
	Tex diffuse;

	Tex specular;
	uint32_t specularStrength;

	Tex metallic;
	uint32_t metallicStrength;
	Tex Roughness;
	uint32_t roughnessStrength;

	Mat() : 
		diffuse("", "material.tDiffuse1"),
		specular("", "material.tSpecular1"),
		metallic("", "material.tMetallic"),
		Roughness("", "material.tRoughness"),
		specularStrength(0),
		metallicStrength(0),
		roughnessStrength(0)
	{
	}

};	





