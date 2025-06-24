#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "appConfig.hpp"
#include "inputManager.hpp"
#include "sceneManager.hpp"

InputManager::InputManager(GLFWwindow* window, Camera& camera) : window(window), camera(camera), m_appConfig(AppConfig::get())
{
    lastX = (float)(m_appConfig.renderWidth / 2);
    lastY = (float)(m_appConfig.renderHeight / 2);
    firstMouse = true;
    mousePosx = 0;
    mousePosy = 0;
}

void InputManager::processInput(float deltaTime, ViewportState viewportState, uint32_t pickingTexture)
{
    wireframeToggleCallback();
    cameraMovementCallback(window, deltaTime, viewportState);
    cameraFocusCallback();
    scrollCallback(viewportState);
    exitCallback();
    pickObjectCallback(viewportState, pickingTexture);
    deleteObjectCallback();
}

void InputManager::cameraFocusCallback()
{
    if (ImGui::IsKeyPressed(ImGuiKey_F, false))
    {
        if (SceneManager::getSelectedPrimitive() != nullptr)
        {
            camera.focusOn(SceneManager::getSelectedPrimitive());
        }
    }
}

void InputManager::deleteObjectCallback()
{
    if ((ImGui::IsKeyPressed(ImGuiKey_X, false) || ImGui::IsKeyPressed(ImGuiKey_Delete, false)) &&
        !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
        !ImGui::IsKeyDown(ImGuiKey_LeftAlt) &&
        !ImGui::IsKeyDown(ImGuiKey_LeftShift))
    {
        if (auto selectedPrimitive = SceneManager::getSelectedPrimitive())
        {
            if (selectedPrimitive->parent)
            {
                selectedPrimitive->parent->removeChild(selectedPrimitive);
            }
        }
    }
}

void InputManager::scrollCallback(ViewportState viewportState)
{
    if (viewportState.mouseWheel != 0.0f && viewportState.isHovered)
    {
        camera.processMouseScroll(viewportState.mouseWheel);
    }
}

void InputManager::pickObjectCallback(ViewportState viewportState, uint32_t pickingTexture)
{
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left, false) && viewportState.isHovered)
    {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        ImVec2 viewportPos = viewportState.position;
        int readX = static_cast<int>(mousePos.x - viewportPos.x);
        // need to flip y axis cuz imgui makes it top left, but opengl uses bottom left
        int readY = static_cast<int>(m_appConfig.renderHeight - (mousePos.y - viewportPos.y));
        int pixel = 0;
        glGetTextureSubImage(pickingTexture, 0, readX, readY, 0, 1, 1, 1, GL_RED_INTEGER, GL_INT, sizeof(pixel),
            &pixel); // pixel == vao id
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
        {
            SceneManager::selectPrimitive(pixel, true); // add to selection
        }
        else
        {
            SceneManager::selectPrimitive(pixel, false); // select new primitive
        }
    }
}

void InputManager::exitCallback()
{
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void InputManager::wireframeToggleCallback()
{
    if (ImGui::IsKeyPressed(ImGuiKey_Z, false) && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
        ImGui::IsKeyDown(ImGuiKey_LeftAlt) && !ImGui::IsKeyDown(ImGuiKey_LeftShift))
    {
        m_appConfig.isWireframe = !m_appConfig.isWireframe;
    }
}

void InputManager::cameraMovementCallback(GLFWwindow* window, float deltaTime, ViewportState viewportState)
{
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
        camera.speed = camera.increasedSpeed;
    else
        camera.speed = camera.defaultSpeed;

    float xPos = ImGui::GetIO().MousePos.x - viewportState.position.x;
    float yPos = ImGui::GetIO().MousePos.y - viewportState.position.y;
    if (firstMouse)
    {
        lastX = (float)xPos;
        lastY = (float)yPos;
        firstMouse = false;
    }
    mousePosx = xPos;
    mousePosy = yPos;

    float xOffset = (float)xPos - (float)lastX;
    float yOffset = lastY - (float)yPos;

    lastX = (float)xPos;
    lastY = (float)yPos;

    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsMouseDown(ImGuiMouseButton_Middle) && viewportState.isHovered)
    {
        camera.processPanning(xOffset, yOffset);
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !ImGui::IsKeyDown(ImGuiKey_LeftShift) && viewportState.isHovered)
    {
        camera.processOrbit(xOffset, yOffset);
    }
}
