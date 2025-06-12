#include "model.hpp"

Scene::Model::Model(Transform transform, std::string name)
{
    this->transform = transform;
    this->name = name;
    this->parent = nullptr;
    this->visible = true;
    this->dirty = false;
    this->movable = true;
    this->children = std::list<std::unique_ptr<SceneNode>>();
    
}
Scene::Model::~Model() = default;

Scene::Model::Model(Model &&other)
{
    this->transform = other.transform;
    this->children = std::move(other.children);
    this->parent = other.parent;
    this->visible = other.visible;
    this->dirty = other.dirty;
    this->movable = other.movable;
    this->name = other.name;
    other.transform = Transform();
    other.parent = nullptr;
    other.visible = false;
    other.dirty = false;
    other.movable = false;
}

Scene::Model &Scene::Model::operator=(Model &&other)
{
    if (this == &other)
        return *this;

    this->transform = other.transform;
    this->children = std::move(other.children);
    this->parent = other.parent;
    this->visible = other.visible;
    this->dirty = other.dirty;
    this->movable = other.movable;
    this->name = other.name;
    other.transform = Transform();
    other.parent = nullptr;
    other.visible = false;
    other.dirty = false;
    other.movable = false;

    return *this;
}

void Scene::Model::draw() const
{
    // Implement the draw logic for the model here
}
void Scene::Model::update() const
{
    // Implement the update logic for the model here
}