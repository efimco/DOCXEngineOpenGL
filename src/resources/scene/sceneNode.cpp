#include "sceneNode.hpp"
#include <iostream>

SceneNode::SceneNode(SceneNode &&other) noexcept
    : transform(other.transform), children(std::move(other.children)), visible(other.visible), dirty(other.dirty),
      movable(other.movable), parent(other.parent)
{
    other.parent = nullptr;
    other.visible = false;
    other.dirty = false;
    other.movable = false;
    other.transform = Transform();
    other.children.clear();
}

SceneNode::SceneNode() : parent(nullptr) { std::cout << "SceneNode created" << std::endl; };

SceneNode::~SceneNode() = default;

void SceneNode::addChild(std::unique_ptr<SceneNode> child)
{
    child->parent = this;
    children.push_back(std::move(child));
}

void SceneNode::removeChild(SceneNode *child)
{
    children.remove_if([child](const std::unique_ptr<SceneNode> &c) { return c.get() == child; });
}

void SceneNode::setParent(SceneNode *newParent)
{
    if (parent)
    {
        parent->removeChild(this);
    }
    if (newParent)
    {
        // We can't use make_unique here since SceneNode is abstract
        parent = newParent;
    }
}
