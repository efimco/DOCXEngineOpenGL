#include "appConfig.hpp"
#include "glad/gl.h"
#include "sceneManager.hpp"
#include "shader.hpp"
#include <filesystem>
#include <string>


std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\multipleLightsSurface.frag").string();
std::string vShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\vertex.vert").string();

std::string fScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.frag").string();
std::string vScreenShader = std::filesystem::absolute("..\\..\\src\\shaders\\frame.vert").string();

std::string fDeferedShader = std::filesystem::absolute("..\\..\\src\\shaders\\defered.frag").string();
std::string vDeferedShader = std::filesystem::absolute("..\\..\\src\\shaders\\defered.vert").string();

namespace AppConfig
{
    int WINDOW_WIDTH = 1024;
    int WINDOW_HEIGHT = 1024;
    int RENDER_WIDTH = 0;
    int RENDER_HEIGHT = 0;
    float clearColor[4] = { 0.133f, 0.192f, 0.265f, 1.0f };
    Shader* baseShader;
    Shader* screenShader;
    Shader* deferedShader;
    float gamma = 1;
    float near_plane = -2.01f;
    float far_plane = 2.0f;
    GLenum polygonMode = GL_FILL;
    bool isWireframe = false;
    bool showObjectPicking = false;
    bool showShadowMap = false;
    bool isFramebufferSizeSetted = true;
    float exposure = 0.1f;
    float irradianceMapIntensity = 1;
    float irradianceMapRotationY = 0;
    std::string cubeMapPath = "";
    bool reloadCubeMap = false;
    float backgroundBlur = 0;

    void initShaders()
    {
        baseShader = new Shader(vShaderPath, fShaderPath);
        SceneManager::addShader(baseShader);
        screenShader = new Shader(vScreenShader, fScreenShader);
        SceneManager::addShader(screenShader);
        deferedShader = new Shader(vDeferedShader, fDeferedShader);
        SceneManager::addShader(deferedShader);
    }

} // namespace AppConfig