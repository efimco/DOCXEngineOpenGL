#pragma once
#include "glm/glm.hpp"

struct Light
{
    int type; // 0 - point, 1 - directional, 2 - spot
    float intensity;
    glm::vec2 stub0; // padding to align to 16 bytes
    glm::vec3 position;
    float stub1;
    glm::vec3 direction;
    float stub2;
    glm::vec3 ambient;
    float stub3;
    glm::vec3 diffuse;
    float stub4;
    glm::vec3 specular;

    // Attenuation
    float constant;
    float linear;
    float quadratic;

    float cutOff;      // inner cone angle
    float outerCutOff; // outer cone angle
};
