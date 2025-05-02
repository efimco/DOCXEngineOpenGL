#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "ImGui/imgui.h"
#include "uiManager.hpp"
#include "sceneManager.hpp"
#include "appConfig.hpp"
#include "inputManager.hpp"
#include "ImGui/imgui_impl_glfw.h"


InputManager::InputManager(GLFWwindow* window, Camera& camera) : window(window), camera(camera) 
{
	lastX = (float)(AppConfig::RENDER_WIDTH / 2);
	lastY = (float) (AppConfig::RENDER_HEIGHT / 2);
	firstMouse = true;
	mousePosx = 0;
	mousePosy = 0;
	wasMouse1Pressed = false;
	wireframeKeyPressed = true;
	rightKeyPressed = false;
}

void InputManager::processInput(float deltaTime)
{
	processWireframeToggleCallback();
	processExitCallback();
	// processObjectPickingCallback(pickingBuffer);
	processCameraMovementCallback(window, deltaTime);
	processCameraResetCallback(window, deltaTime);
}

void InputManager::scrollCallback()
{
	float yOffset = ImGui::GetIO().MouseWheel;
	camera.processMouseScroll(yOffset);	
}

void InputManager::mouseCallback()
{
	float xPos = ImGui::GetIO().MousePos.x;
	float yPos = ImGui::GetIO().MousePos.y;
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
	if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		camera.processMouseMovement(xOffset,yOffset);

}

void InputManager::processExitCallback()
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window,true);

}

void InputManager::processWireframeToggleCallback()
{
	if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !wireframeKeyPressed)
	{
		wireframeKeyPressed = true;
		AppConfig::isWireframe = !AppConfig::isWireframe;
	}

	if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
		wireframeKeyPressed = false;
}

void InputManager::processObjectPickingCallback(PickingBuffer& pickingBuffer)
{
	if( AppConfig::RENDER_WIDTH != 0 && AppConfig::RENDER_HEIGHT != 0) 
	{
		if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
			wasMouse1Pressed = true;
		else if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && wasMouse1Pressed)
		{
			wasMouse1Pressed = false;
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			projection = glm::perspective(glm::radians(camera.zoom), float(AppConfig::RENDER_WIDTH)/float(AppConfig::RENDER_HEIGHT),0.1f, 100.0f);    
			view = camera.getViewMatrix();
			glm::vec3 pickedColor = pickingBuffer.pickColorAt(mousePosx, mousePosy, AppConfig::RENDER_HEIGHT);
			Primitive* primitive = pickingBuffer.getIdFromPickColor(pickedColor);
			if (primitive != nullptr)
			{
				if (SceneManager::getSelectedPrimitive() != primitive)
				{
					if (SceneManager::getSelectedPrimitive() != nullptr) SceneManager::getSelectedPrimitive()->selected = false;
					primitive->selected = true;
					SceneManager::setSelectedPrimitive(primitive);
				}
				std::cout << "VAO: " << primitive->vao << std::endl;
			}
			else
			{
				if (SceneManager::getSelectedPrimitive() != nullptr)
				{
					SceneManager::getSelectedPrimitive()->selected = false;
					SceneManager::setSelectedPrimitive(nullptr);
				}
			}
		}
	}
}

void InputManager::processCameraMovementCallback(GLFWwindow *window, float deltaTime)
{
	if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
	{
		if (!rightKeyPressed)
		{
			rightKeyPressed = true;
			glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
		}
		camera.cameraReseted = true;

		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.processKeyboard(FORWARD, deltaTime);

		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.processKeyboard(BACKWARD, deltaTime);


		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.processKeyboard(LEFT, deltaTime);

		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.processKeyboard(RIGHT, deltaTime);

		if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			camera.processKeyboard(DOWN, deltaTime);

		if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			camera.processKeyboard(UP, deltaTime);
	}
	else
	{
		if (rightKeyPressed) 
			glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_NORMAL);
		rightKeyPressed = false;
	} 

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.speed = camera.increasedSpeed;
	else 
		camera.speed = camera.defaultSpeed;

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		camera.cameraReseted = false;
}

void InputManager::processCameraResetCallback(GLFWwindow *window, float deltaTime)
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
