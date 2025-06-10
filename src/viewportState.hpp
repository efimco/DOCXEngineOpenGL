#pragma once
#include "ImGui/imgui.h"
#include <glm/glm.hpp>

struct ViewportState
{
    ImVec2 position;          // Viewport position in screen space
    ImVec2 size;              // Viewport size
    bool isHovered;           // Whether viewport is currently hovered
    bool wantCaptureMouse;    // Whether ImGui wants to capture mouse input
    bool wantCaptureKeyboard; // Whether ImGui wants to capture keyboard input
    float mouseWheel;
    ImVec2 cursorPos;

    bool contains(float x, float y) const
    {
        return x >= position.x && x <= position.x + size.x && y >= position.y && y <= position.y + size.y;
    }

    // Get normalized coordinates within viewport (0-1)
    glm::vec2 getNormalizedCoordinates(float x, float y) const
    {
        return glm::vec2((x - position.x) / size.x, (y - position.y) / size.y);
    }
};
