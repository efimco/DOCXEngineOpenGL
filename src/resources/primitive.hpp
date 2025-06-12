#pragma once
#include "material.hpp"
#include "scene/sceneNode.hpp"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <string>

class Primitive : public SceneNode
{
  public:
    uint32_t vao;
    std::pair<glm::vec3, glm::vec3> boundingBox; // minimum and maximum value

    std::shared_ptr<Mat> material;

    Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, size_t indexCount, Transform transform,
              std::pair<glm::vec3, glm::vec3> boundingBox, std::shared_ptr<Mat> material, std::string name);
    Primitive(const Primitive &other);
    Primitive &operator=(const Primitive &) = delete;
    Primitive(Primitive &&other) noexcept;
    Primitive &operator=(Primitive &&other) noexcept;
    ~Primitive() override;
    void draw() const override;
    void update() const override;

  private:
    uint32_t vbo;
    uint32_t ebo;
    size_t indexCount;
};