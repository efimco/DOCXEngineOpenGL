#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"

#include "sceneManager.hpp"
#include "appConfig.hpp"
#include "inputManager.hpp"


InputManager::InputManager(GLFWwindow* window, Camera& camera) : window(window), camera(camera) 
{
	lastX = (float)(AppConfig::RENDER_WIDTH / 2);
	lastY = (float) (AppConfig::RENDER_HEIGHT / 2);
	firstMouse = true;
	mousePosx = 0;
	mousePosy = 0;
}

void InputManager::processInput(float deltaTime, ViewportState viewportState)
{
	wireframeToggleCallback();
	cameraMovementCallback(window, deltaTime);
	cameraResetCallback(window, deltaTime);
	scrollCallback(viewportState);
	exitCallback();
}

void InputManager::scrollCallback(ViewportState viewportState)
{
	if (viewportState.mouseWheel != 0.0f)
	{
		std::cout << "Scroll yOffset: " << viewportState.mouseWheel << std::endl;
		camera.processMouseScroll(viewportState.mouseWheel);
	}
}

void InputManager::exitCallback()
{
	if(ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
	glfwSetWindowShouldClose(window,true);
	}
}

void InputManager::wireframeToggleCallback()
{
	if(ImGui::IsKeyPressed(ImGuiKey_LeftCtrl, false))
	{
		AppConfig::isWireframe = !AppConfig::isWireframe;
	}
}

void InputManager::cameraMovementCallback(GLFWwindow *window, float deltaTime)
{
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		camera.cameraReseted = true;

		if(ImGui::IsKeyDown(ImGuiKey_W)) camera.processKeyboard(FORWARD, deltaTime);
		if(ImGui::IsKeyDown(ImGuiKey_S)) camera.processKeyboard(BACKWARD, deltaTime);
		if(ImGui::IsKeyDown(ImGuiKey_A)) camera.processKeyboard(LEFT, deltaTime);
		if(ImGui::IsKeyDown(ImGuiKey_D)) camera.processKeyboard(RIGHT, deltaTime);
		if(ImGui::IsKeyDown(ImGuiKey_Q)) camera.processKeyboard(DOWN, deltaTime);
		if(ImGui::IsKeyDown(ImGuiKey_E)) camera.processKeyboard(UP, deltaTime);
	}

	if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
		camera.speed = camera.increasedSpeed;
	else 
		camera.speed = camera.defaultSpeed;
	
	float xPos = ImGui::GetIO().MousePos.x - m_windowPos.x;
	float yPos = ImGui::GetIO().MousePos.y - m_windowPos.y;
	if (firstMouse)
	{	lastX = (float)xPos;
		lastY = (float)yPos;
		firstMouse = false;
	}
	mousePosx = xPos;
	mousePosy = yPos;
	
	float xOffset = (float)xPos - (float)lastX;
	float yOffset = lastY - (float)yPos ;
	
	lastX = (float)xPos;
	lastY = (float)yPos;
	if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		camera.processMouseMovement(xOffset, yOffset);
	}

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
	if(!camera.cameraReseted && (glm::length(camera.position - camera.defaultCameraMatrix[0]) > .05 ||
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
	else camera.cameraReseted = true; 
}

