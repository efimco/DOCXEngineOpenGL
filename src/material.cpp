
#include "material.hpp"
#include "texture.hpp"


Mat::Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular) :
	name(Name),
	diffuse(diffuse),
	specular(Specular),
	metallic(std::make_shared<Tex>("", "tMetallic")),
	roughness(std::make_shared<Tex>("", "tRoughness")),
	normal(std::make_shared<Tex>("", "tNormal")),
	specularStrength(1),
	metallicStrength(1),
	roughnessStrength(1)
{}

Mat::Mat() :
	diffuse(std::make_shared<Tex>("", "tDiffuse")),
	normal(std::make_shared<Tex>("", "tNormal")),
	specular(std::make_shared<Tex>("", "tSpecular")),
	metallic(std::make_shared<Tex>("", "material.tMetallic")),
	roughness(std::make_shared<Tex>("", "material.tRoughness")),
	specularStrength(0),
	metallicStrength(0),
	roughnessStrength(0)
{};






