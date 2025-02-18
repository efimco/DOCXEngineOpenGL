#pragma once
#include <string>
#include "tiny_gltf.h"
#include "stb_image.h"

struct Tex
{
	uint32_t id;
	std::string type;
	std::string path;

	Tex() : id(-1), type(""), path("") {};
	Tex(const char* path, std::string type):path(path), type(type)
	{
		if (path != "")
		{
			id = TextureFromFile(path);
			std::cout << "Loaded texture: " << path << std::endl;
		}
		else id = 0;
	};
	Tex(tinygltf::Image& image, std::string type):path(path), type(type)
	{
		id = TextureFromGltf(image);
		std::cout << "Loaded texture: " << image.name << std::endl;
		this->path = image.name;
	};
	~Tex(){};

	void SetPath(const std::string& newPath) 
	{
		path = newPath;
		id = TextureFromFile(newPath.c_str());
	}
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

	uint32_t TextureFromGltf(tinygltf::Image& image)
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
};


struct Mat {
	std::string name;
	std::shared_ptr<Tex> diffuse;
	std::shared_ptr<Tex> specular;
	uint32_t specularStrength;

	std::shared_ptr<Tex> metallic;
	uint32_t metallicStrength;
	std::shared_ptr<Tex> roughness;
	uint32_t roughnessStrength;

	Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular) :
		name(Name),
		diffuse(diffuse),
		specular(Specular),
		metallic(std::make_shared<Tex>("", "tMetallic")),
		roughness(std::make_shared<Tex>("", "tRoughness")),
		specularStrength(1),
		metallicStrength(1),
		roughnessStrength(1)
	{}

	Mat() :
		diffuse(std::make_shared<Tex>("", "tDiffuse")),
		specular(std::make_shared<Tex>("", "tSpecular")),
		metallic(std::make_shared<Tex>("", "material.tMetallic")),
		roughness(std::make_shared<Tex>("", "material.tRoughness")),
		specularStrength(0),
		metallicStrength(0),
		roughnessStrength(0)
	{}
};





