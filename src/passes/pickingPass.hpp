#include "shader.hpp"
#include <cstdint>
#include <glm/glm.hpp>

class PickingPass
{
  public:
    PickingPass();
    void draw(glm::mat4 projection, glm::mat4 view);
    void createOrResize();
    uint32_t pickingTexture;
    Shader *pickingShader;
    uint32_t m_pickingFBO;

  private:
    uint32_t m_pickingRBO;

    void initTextures();
};