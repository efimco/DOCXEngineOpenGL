#pragma once
#include "camera.hpp"
#include "glm/glm.hpp"
#include "shader.hpp"
#include <cstdint>


class ShadowMap
{
  public:
    const int width;
    const int height;
    uint32_t depthMapFBO;
    uint32_t depthMap;
    Shader *depthShader;

    ShadowMap(const int width, const int height);
    ~ShadowMap();

    void draw(Camera &camera);
};