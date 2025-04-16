#pragma once
#include <glm/glm.hpp>
#include <string>
#include <GLFW/glfw3.h>
#include "camera.hpp"

class UIManager 
{
	public:
		float deltaTime;
		Camera& camera;
		GLFWwindow* window;
		UIManager(GLFWwindow* window, float deltaTime, Camera& camera);
		~UIManager();

		void draw();
		void showFps();
		void showCameraTransforms();
		void showLights();
		void showObjectInspector();
		void showTools();
		void showMaterialBrowser();
		bool wantCaptureInput() const;

	private:
		std::string OpenFileDialog();

};