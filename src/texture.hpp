#pragma once
#include <string>
#include "tiny_gltf.h"

class Tex
{
	public:
	uint32_t id;
	std::string type;
	std::string path;

	Tex(const char* path, std::string type);
	Tex();
	Tex(tinygltf::Image& image, std::string type);
	~Tex();

	void SetPath(const std::string& newPath);
	uint32_t TextureFromFile(const char *path);
	uint32_t TextureFromGlb(tinygltf::Image& image);
};