
#include "material.hpp"
#include "texture.hpp"

Mat::Mat(std::string Name, std::shared_ptr<Tex> diffuse, std::shared_ptr<Tex> Specular)
    : name(Name), diffuse(diffuse), specular(Specular), metallic(0), roughness(0.04f), normal(std::make_shared<Tex>(""))
{
}

Mat::~Mat()
{
    diffuse.reset();
    specular.reset();
    normal.reset();
}

Mat::Mat()
    : diffuse(std::make_shared<Tex>("")), normal(std::make_shared<Tex>("")), specular(std::make_shared<Tex>("")),
      metallic(0), roughness(0.04f) {};