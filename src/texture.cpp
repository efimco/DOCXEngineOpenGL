#include <iostream>
#include "glad/glad.h"
#include "stb_image.h"
#include "texture.hpp"



Tex::Tex() : id(-1), type(""), path("") {};

Tex::Tex(const char* path, std::string type):path(path), type(type)
{
	if (path != "")
	{
		id = TextureFromFile(path);
		std::cout << "Loaded texture: " << path << std::endl;
	}
	else id = 0;
};

Tex::Tex(tinygltf::Image& image, std::string type):path(path), type(type)
{
	id = TextureFromGltf(image);
	std::cout << "Loaded texture: " << image.name << std::endl;
	this->path = image.name;
};

Tex::~Tex(){};

void Tex::SetPath(const std::string& newPath) 
{
	path = newPath;
	id = TextureFromFile(newPath.c_str());
}

uint32_t Tex::TextureFromFile(const char *path)
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
uint32_t Tex::TextureFromGltf(tinygltf::Image& image)
{
	GLenum format;
	if (image.component == 1)
		format = GL_RED;
	else if (image.component == 3)
		format = GL_RGB;
	else if (image.component == 4)
		format = GL_RGBA;

	uint32_t textureID;
	glGenTextures(1, &textureID);	
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.image.data());
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return textureID;
}