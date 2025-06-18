#include "sceneNode.hpp"
#include <memory_resource>

class Baker : public SceneNode
{
private:
	inline static std::pmr::unsynchronized_pool_resource bakerMemoryResource{};
public:
	Baker();
	~Baker() override final;

	void bake();
};