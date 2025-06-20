
#include "material.hpp"
#include "texture.hpp"

Mat::Mat(std::string Name, std::shared_ptr<Tex> Diffuse, std::shared_ptr<Tex> Specular)
	: name(Name),
	tDiffuse(Diffuse),
	tSpecular(Specular),
	metallic(1.0f),
	roughness(0.5f),
	albedo(glm::vec4(.5, .5, .5, 1.0f)),
	tNormal(std::make_shared<Tex>(""))
{
}

Mat::~Mat()
{
	tDiffuse.reset();
	tSpecular.reset();
	tNormal.reset();
}

Mat::Mat()
	: tDiffuse(std::make_shared<Tex>("")),
	tNormal(std::make_shared<Tex>("")),
	tSpecular(std::make_shared<Tex>("")),
	albedo(glm::vec4(.5, .5, .5, 1.0f)),
	metallic(1.0f), roughness(0.5f)
{
};
