#pragma once
#include "sceneNode.hpp"
namespace Scene
{
class Model : public SceneNode
{
  public:
    Model(Transform transform);
    ~Model() override;
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
    Model(Model &&other);
    Model &operator=(Model &&other);
    void draw() const override;
    void update() const override;
};
} // namespace Scene