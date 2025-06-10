#include <glad/glad.h>  // Include GLAD before GLFW
#include <GLFW/glfw3.h>
#include <iostream>

#include "appConfig.hpp"
#include "renderer.hpp"

void framebufferSizeCallback(GLFWwindow *window, int32_t newWINDOW_WIDTH, int32_t newWINDOW_HEIGHT)
{
    AppConfig::WINDOW_WIDTH = newWINDOW_WIDTH;
    AppConfig::WINDOW_HEIGHT = newWINDOW_HEIGHT;
    AppConfig::isFramebufferSizeSetted = false;
}

int main()
{
    GLFWwindow *window;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    // glfwWindowHint(GLFW_FLOATING, GLFW_TRUE); // Uncomment this line to make the window floating
    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    window = glfwCreateWindow(AppConfig::WINDOW_WIDTH, AppConfig::WINDOW_HEIGHT, "Main Window", NULL, NULL);
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
