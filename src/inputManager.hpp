#pragma once
#include "ImGui/imgui.h"
#include "camera.hpp"
#include "viewportState.hpp"
#include <GLFW/glfw3.h>


class InputManager
{
  public:
    InputManager(GLFWwindow *window, Camera &camera);
    ~InputManager() = default;

    void processInput(float deltaTime, ViewportState viewportState, uint32_t pickingTexture);

  private:
    GLFWwindow *window;
    Camera &camera;

    float lastX;
    float lastY;
    bool firstMouse;
    double mousePosx;
    double mousePosy;

    void scrollCallback(ViewportState viewportState);
    void cameraFocusCallback();
    void pickObjectCallback(ViewportState viewportState, uint32_t pickingTexture);
    void exitCallback();
    void cameraMovementCallback(GLFWwindow *window, float deltaTime, ViewportState viewportState);
    void wireframeToggleCallback();
};
