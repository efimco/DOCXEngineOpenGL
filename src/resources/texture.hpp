#pragma once
#include "tiny_gltf.h"
#include <string>


class Tex
{
public:
	uint32_t id;
	std::string path;
	bool tiled;
	void setTiled(bool tiled);

	Tex(const char* path);
	Tex();
	Tex(tinygltf::Image& image);
	~Tex();

	void setPath(const std::string& newPath);

private:
	uint32_t TextureFromFile(const char* path);
	uint32_t TextureFromGlb(tinygltf::Image& image);
};