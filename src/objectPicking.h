#include "glm/glm.hpp"
#include <vector>

glm::vec3 GetRayFromMouse(double mouseX, double mouseY, int screenWidth, int screenHeight, const glm::mat4 &projection, const glm::mat4 &view) {
    // Convert mouse screen coordinates to normalized device coordinates (NDC)
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight; // Flip Y axis
    glm::vec4 rayNDC(x, y, -1.0f, 1.0f);

    // Convert to camera space (unprojecting using the inverse projection matrix)
    glm::vec4 rayClip = glm::inverse(projection) * rayNDC;
    rayClip = glm::vec4(rayClip.x, rayClip.y, -1.0f, 0.0f); // Set depth to -1 (pointing forward)

    // Convert to world space (unproject using inverse view matrix)
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayClip);
    rayWorld = glm::normalize(rayWorld); // Normalize the direction

    return rayWorld;
}

bool RayIntersectsAABB(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const glm::vec3 &minBounds, const glm::vec3 &maxBounds, float &t) {
    float tMin = (minBounds.x - rayOrigin.x) / rayDir.x;
    float tMax = (maxBounds.x - rayOrigin.x) / rayDir.x;
    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (minBounds.y - rayOrigin.y) / rayDir.y;
    float tyMax = (maxBounds.y - rayOrigin.y) / rayDir.y;
    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax)) return false;
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    float tzMin = (minBounds.z - rayOrigin.z) / rayDir.z;
    float tzMax = (maxBounds.z - rayOrigin.z) / rayDir.z;
    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax)) return false;
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;

    t = tMin;
    return true;
}

struct Object {
    glm::vec3 minBounds, maxBounds;
    uint32_t shaderProgram; // Store the shader program ID
};

// Example function to get the shader of the hit object
uint32_t GetShaderFromRaycast(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const std::vector<Object> &objects) {
    float closestT = FLT_MAX;
    uint32_t hitShader = 0;

    for (const auto &obj : objects) {
        float t;
        if (RayIntersectsAABB(rayOrigin, rayDir, obj.minBounds, obj.maxBounds, t)) {
            if (t < closestT) {
                closestT = t;
                hitShader = obj.shaderProgram;
            }
        }
    }

    return hitShader; // Return the shader of the closest hit object
}

