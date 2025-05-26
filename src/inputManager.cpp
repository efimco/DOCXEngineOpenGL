#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include <glad/gl.h>
#include "sceneManager.hpp"
#include "appConfig.hpp"
#include "inputManager.hpp"

InputManager::InputManager(GLFWwindow *window, Camera &camera) : window(window), camera(camera)
{
	lastX = (float)(AppConfig::RENDER_WIDTH / 2);
	lastY = (float)(AppConfig::RENDER_HEIGHT / 2);
	firstMouse = true;
	mousePosx = 0;
	mousePosy = 0;
}

void InputManager::processInput(float deltaTime, ViewportState viewportState, uint32_t pickingTexture)
{
	wireframeToggleCallback();
	cameraMovementCallback(window, deltaTime, viewportState);
	cameraResetCallback(window, deltaTime);
	scrollCallback(viewportState);
	exitCallback();
	pickObjectCallback(viewportState, pickingTexture);
}

void InputManager::scrollCallback(ViewportState viewportState)
{
	if (viewportState.mouseWheel != 0.0f)
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
		int readY = static_cast<int>(AppConfig::RENDER_HEIGHT - (mousePos.y - viewportPos.y));
		int pixel = 0;
		glGetTextureSubImage(pickingTexture, 0, readX, readY, 0, 1, 1, 1, GL_RED_INTEGER, GL_INT, sizeof(pixel), &pixel); // pixel == vao id
		SceneManager::selectPrimitive(pixel);
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
	if (ImGui::IsKeyPressed(ImGuiKey_LeftCtrl, false))
	{
		AppConfig::isWireframe = !AppConfig::isWireframe;
	}
}

void InputManager::cameraMovementCallback(GLFWwindow *window, float deltaTime, ViewportState viewportState)
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

	if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsMouseDown(ImGuiMouseButton_Middle))
	{
		camera.processPanning(xOffset, yOffset);
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !ImGui::IsKeyDown(ImGuiKey_LeftShift))
	{
		camera.processOrbit(xOffset, yOffset);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_F, false))
	{
		camera.cameraReseted = false;
	}
}

void InputManager::cameraResetCallback(GLFWwindow *window, float deltaTime)
{
	if (!camera.cameraReseted && (glm::length(camera.position - camera.defaultCameraMatrix[0]) > .05 ||
								  glm::length(camera.front - camera.defaultCameraMatrix[1]) > .05 ||
								  glm::length(camera.up - camera.defaultCameraMatrix[2]) > .05 ||
								  glm::abs(camera.pitch - camera.defaultCameraRotation[0]) > .05 ||
								  glm::abs(camera.yaw - camera.defaultCameraRotation[1]) > .05))
	{
		float speed = 10 * deltaTime;
		camera.position = glm::mix(camera.position, camera.defaultCameraMatrix[0], speed);
		camera.front = glm::mix(camera.front, camera.defaultCameraMatrix[1], speed);
		camera.up = glm::mix(camera.up, camera.defaultCameraMatrix[2], speed);
		camera.pitch = glm::mix(camera.pitch, camera.defaultCameraRotation[0], speed);
		camera.yaw = glm::mix(camera.yaw, camera.defaultCameraRotation[1], speed);
	}
	else
		camera.cameraReseted = true;
}
