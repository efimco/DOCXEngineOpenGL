#include <glad/glad.h>  // Include GLAD before GLFW
#include <GLFW/glfw3.h>
#include <iostream>

#include "appConfig.hpp"
#include "renderer.hpp"

#ifdef _MSC_VER
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
#endif

void framebufferSizeCallback(GLFWwindow *window, int32_t newWINDOW_WIDTH, int32_t newWINDOW_HEIGHT)
{
	
	AppConfig::get().windowWidth = newWINDOW_WIDTH;
	AppConfig::get().windowWidth += newWINDOW_WIDTH % 2; // Ensure width is even
	AppConfig::get().windowHeight = newWINDOW_WIDTH;
	AppConfig::get().windowHeight += newWINDOW_HEIGHT % 2;; // Ensure height is even
	AppConfig::get().isFramebufferSizeSet = false;
}

int main()
{
#ifdef _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	GLFWwindow *window;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // Uncomment this line to make the window floating
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    AppConfig::get().windowWidth = AppConfig::get().windowHeight = 1024;
	window = glfwCreateWindow(AppConfig::get().windowWidth, AppConfig::get().windowHeight, "Main Window", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	Renderer renderer(window);
	renderer.render(window);

	glfwTerminate();
	return 0;
}
