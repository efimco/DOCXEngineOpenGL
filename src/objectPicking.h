#pragma once
#include "glm/glm.hpp"
#include <vector>
#include "primitive.h"
glm::vec3 ScreenPosToWorldRay(
    double mouseX, double mouseY, int screenWidth, int screenHeight,
    glm::mat4 projection, glm::mat4 view)
{
    // Преобразование координат мыши в диапазон [-1, 1]
    double x = (2.0f * mouseX) / screenWidth - 1.0f;
    double y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

    // Преобразуем в координаты вида
    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    // Преобразуем в мировые координаты
    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    return glm::normalize(ray_world);
}

// Проверка пересечения луча со сферой (грубая проверка)
bool RayIntersectsSphere(glm::vec3 rayOrigin, glm::vec3 rayDir, const Primitive& obj) {
    glm::vec3 oc = rayOrigin - glm::vec3(obj.transform[3]);
    double b = 2.0f * glm::dot(oc, rayDir);
    double c = glm::dot(oc, oc) - .1 * .1;
    double discriminant = b * b - 4 * c;
    return (discriminant > 0);
}

Primitive* PickObject(
    double mouseX, double mouseY, int screenWidth, int screenHeight,
    glm::mat4 projection, glm::mat4 view, glm::vec3 cameraPos, std::vector<Primitive>& objects)
{
    glm::vec3 rayDir = ScreenPosToWorldRay(mouseX, mouseY, screenWidth, screenHeight, projection, view);

    Primitive* closestObject = nullptr;
    float closestDist = FLT_MAX;

    for (auto& obj : objects) {
        if (RayIntersectsSphere(cameraPos, rayDir, obj)) {
            float dist = glm::length(glm::vec4(cameraPos,1) - obj.transform[3]);
            if (dist < closestDist) {
                closestDist = dist;
                closestObject = &obj;
            }
        }
    }

    return closestObject;
}