#include "transform.hpp"
#include <glm/glm.hpp>
#include <list>
#include <memory>

class SceneNode
{
  public:
    Transform transform;
    std::list<std::unique_ptr<SceneNode>> children;
    SceneNode *parent = nullptr;
    bool visible = true;
    bool dirty = false;
    bool movable = true;
    SceneNode();
    SceneNode(const SceneNode &) = delete;
    SceneNode &operator=(const SceneNode &) = delete;
    SceneNode(SceneNode &&other) noexcept;
    virtual ~SceneNode();
    virtual void draw() const = 0;
    virtual void update() const = 0;
    void addChild(std::unique_ptr<SceneNode> child);
    void removeChild(SceneNode *child);
    void setParent(SceneNode *newParent);
};