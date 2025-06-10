#include "model.hpp"

Scene::Model::Model(Transform transform) { SceneNode::transform = transform; }
Scene::Model::~Model() = default;

Scene::Model::Model(Model &&other)
{
    SceneNode::transform = other.SceneNode::transform;
    other.SceneNode::transform = Transform();
    SceneNode::children = std::move(other.SceneNode::children);
    SceneNode::parent = other.SceneNode::parent;
    SceneNode::visible = other.SceneNode::visible;
    SceneNode::dirty = other.SceneNode::dirty;
    SceneNode::movable = other.SceneNode::movable;
    other.SceneNode::parent = nullptr;
    other.SceneNode::visible = false;
    other.SceneNode::dirty = false;
    other.SceneNode::movable = false;
}

Scene::Model &Scene::Model::operator=(Model &&other)
{
    if (this == &other)
        return *this;

    SceneNode::transform = other.SceneNode::transform;
    other.SceneNode::transform = Transform();
    SceneNode::children = std::move(other.SceneNode::children);
    SceneNode::parent = other.SceneNode::parent;
    SceneNode::visible = other.SceneNode::visible;
    SceneNode::dirty = other.SceneNode::dirty;
    SceneNode::movable = other.SceneNode::movable;
    other.SceneNode::parent = nullptr;
    other.SceneNode::visible = false;
    other.SceneNode::dirty = false;
    other.SceneNode::movable = false;

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