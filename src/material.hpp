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

	float metallic;
	float roughness;

	std::shared_ptr<Tex> normal;

	Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular);

	~Mat();

	Mat();
};