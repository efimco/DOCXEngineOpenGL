#include <iostream>
#include "glad/gl.h"
#include "stb_image.h"
#include "texture.hpp"
#include "sceneManager.hpp"

Tex::Tex() : id(-1), path("") {};

Tex::Tex(const char *path) : path(path)
{
	if (path && path[0] != '\0')
	{
		id = TextureFromFile(path);
		std::cout << "Loaded texture from file: " << path << std::endl;
	}
	else
		id = 0;
};

Tex::Tex(tinygltf::Image &image) : path(path)
{
	id = TextureFromGlb(image);
	std::cout << "Loaded texture from gltf: " << image.name << std::endl;
	this->path = image.name;
};

Tex::~Tex()
{
	glDeleteTextures(1, &id);
	this->id = -1;
};

void Tex::setPath(const std::string &newPath)
{
	auto &cache = SceneManager::getTextureCache();
	auto it = cache.find(path);
	if (it != cache.end())
	{
		cache[newPath] = it->second;
		cache.erase(it);
	}
	path = newPath;
	id = TextureFromFile(newPath.c_str());
}

uint32_t Tex::TextureFromFile(const char *path)
{
	std::string filename = std::string(path);
	uint32_t textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		GLenum internalFormat;
		if (nrComponents == 1)
		{
			internalFormat = GL_R8;
			format = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = GL_RGB8;
			format = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = GL_RGBA8;
			format = GL_RGBA;
		}

		int nMipLevels = (int)floor(log2(std::max(width, height))) + 1;
		glTextureStorage2D(textureID, nMipLevels, internalFormat, width, height);
		glTextureSubImage2D(textureID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

		glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateTextureMipmap(textureID);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

uint32_t Tex::TextureFromGlb(tinygltf::Image &image)
{
	GLenum format;
	GLenum internalFormat;
	if (image.component == 1)
	{
		internalFormat = GL_R8;
		format = GL_RED;
	}
	else if (image.component == 3)
	{
		internalFormat = GL_RGB8;
		format = GL_RGB;
	}
	else if (image.component == 4)
	{
		internalFormat = GL_RGBA8;
		format = GL_RGBA;
	}

	uint32_t textureID;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

	int nMipLevels = (int)floor(log2(std::max(image.width, image.height))) + 1;
	glTextureStorage2D(textureID, nMipLevels, internalFormat, image.width, image.height);
	glTextureSubImage2D(textureID, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, image.image.data());

	glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateTextureMipmap(textureID);

	return textureID;
}