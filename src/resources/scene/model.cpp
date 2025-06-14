#include "model.hpp"

SceneGraph::Model::Model(Transform transform, std::string name)
{
    this->transform = transform;
    this->name = name;
    this->parent = nullptr;
    this->visible = true;
    this->dirty = false;
    this->movable = true;
    this->children = std::vector<std::unique_ptr<SceneNode>>();
}
SceneGraph::Model::~Model() = default;

SceneGraph::Model::Model(Model &&other)
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

SceneGraph::Model &SceneGraph::Model::operator=(Model &&other)
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

void *SceneGraph::Model::operator new(size_t size)
{
    return modelMemoryResource.allocate(size, alignof(SceneGraph::Model));
}   

void SceneGraph::Model::operator delete(void *ptr) noexcept
{
    modelMemoryResource.deallocate(ptr, sizeof(Model), alignof(SceneGraph::Model));
}