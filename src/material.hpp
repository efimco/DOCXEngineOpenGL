#pragma once
#include <string>
#include "texture.hpp"
#include <memory>


class Mat 
{
public:
	std::string name;
	std::shared_ptr<Tex> diffuse;
	std::shared_ptr<Tex> specular;
	uint32_t specularStrength;

	std::shared_ptr<Tex> metallic;
	uint32_t metallicStrength;
	std::shared_ptr<Tex> roughness;
	uint32_t roughnessStrength;

	std::shared_ptr<Tex> normal;

	Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular);

	Mat();
};





