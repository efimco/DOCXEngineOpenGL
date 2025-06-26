#pragma once
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::mat4 matrix;
    glm::mat4 prevMatrix;
};