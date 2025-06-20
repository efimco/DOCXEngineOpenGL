#pragma once
#include "texture.hpp"
#include <memory>
#include <string>
#include <glm/glm.hpp>


class Mat
{
public:
	std::string name;
	std::shared_ptr<Tex> tDiffuse;
	std::shared_ptr<Tex> tSpecular;
	glm::vec4 albedo;

	float metallic;
	float roughness;

	std::shared_ptr<Tex> tNormal;

	Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular);

	~Mat();

	Mat();
};