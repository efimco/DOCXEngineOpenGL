#pragma once
#include "sceneNode.hpp"
#include <memory_resource>
#include <string>

namespace SceneGraph
{
class Model final : public SceneNode
{
  public:
    inline static std::pmr::monotonic_buffer_resource modelMemoryResource{};
    Model(Transform transform, std::string name);
    ~Model() override final;
    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;
    Model(Model &&other);
    Model &operator=(Model &&other);
    void *operator new(size_t size);
    void operator delete(void *ptr) noexcept;
};
} // namespace SceneGraph