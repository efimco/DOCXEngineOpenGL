#pragma once
#include "ImGui/imgui.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


#include "camera.hpp"
#include "inputManager.hpp"
#include "viewportState.hpp"


class UIManager
{
  public:
    UIManager(GLFWwindow *window, Camera &camera);
    ~UIManager();

    Camera &camera;
    GLFWwindow *window;

    bool viewportHovered;
    bool viewportSizeSetteled;

    ViewportState getViewportState();
    glm::vec2 getViewportSize();

    void setScreenTexture(uint32_t texId);
    void setPickingTexture(uint32_t texId);
    void setShadowMapTexture(uint32_t texId);
    void draw(float deltaTime);

  private:
    ViewportState m_viewportState;
    ImVec2 m_viewportPos;

    uint32_t m_screenTexture;
    uint32_t m_pickingTexture;
    uint32_t m_shadowMapTexture;
    ImVec2 m_vpSize;

    std::string OpenFileDialog();

    void showCameraTransforms();
    void showLights();
    void showObjectInspector();
    void showTools();
    void showMaterialBrowser();
    void getScroll();
    void getViewportPos();
    void getCursorPos();
    void showViewport(float deltaTime);
};