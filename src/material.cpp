
#include "material.hpp"
#include "texture.hpp"


Mat::Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular) :
	name(Name),
	diffuse(diffuse),
	specular(Specular),
	metallic(std::make_shared<Tex>("")),
	roughness(std::make_shared<Tex>("")),
	normal(std::make_shared<Tex>("")),
	specularStrength(1),
	metallicStrength(1),
	roughnessStrength(1)
{}

Mat::Mat() :
	diffuse(std::make_shared<Tex>("")),
	normal(std::make_shared<Tex>("")),
	specular(std::make_shared<Tex>("")),
	metallic(std::make_shared<Tex>("")),
	roughness(std::make_shared<Tex>("")),
	specularStrength(0),
	metallicStrength(0),
	roughnessStrength(0)
{};






