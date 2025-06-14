#include "sceneNode.hpp"
#include <iostream>
#include "sceneManager.hpp"

SceneNode::SceneNode(SceneNode&& other) noexcept
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

void SceneNode::addChild(std::unique_ptr<SceneNode>&& child)
{
	child->parent = this;
	children.push_back(std::move(child));
}

void SceneNode::removeChild(SceneNode* child)
{
	if (auto* primitive = dynamic_cast<Primitive*>(child))
	{
		SceneManager::removePrimitive(primitive);
	}
	if (child->parent == this)
	{
		child->parent = nullptr;
		auto it = std::remove_if(children.begin(), children.end(),
			[child](const std::unique_ptr<SceneNode>& c) { return c.get() == child; });
		if (it != children.end())
		{
			children.erase(it, children.end());
		}
		else
		{
			std::cerr << "Error: Child not found in parent's children list." << std::endl;
		}
	}
}
