#include "sceneNode.hpp"
#include <iostream>

SceneNode::SceneNode(SceneNode &&other) noexcept
    : transform(other.transform), children(std::move(other.children)), visible(other.visible), dirty(other.dirty),
      movable(other.movable)
{
    this->parent = other.parent;
    other.parent = nullptr;
    other.visible = false;
    other.dirty = false;
    other.movable = false;
    other.transform = Transform();
    other.children.clear();
}

SceneNode::SceneNode() : parent(nullptr) { std::cout << "SceneNode created" << std::endl; };

SceneNode::~SceneNode() = default;

void SceneNode::addChild(std::unique_ptr<SceneNode> &&child)
{
    child->parent = this;
    children.push_back(std::move(child));
}
