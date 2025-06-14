#include "scene.hpp"

Scene::Scene(std::string name)
{
    this->name = std::move(name);
    this->parent = nullptr;
    this->visible = true;
    this->dirty = false;
    this->movable = true;
    this->children = std::vector<std::unique_ptr<SceneNode>>();
}