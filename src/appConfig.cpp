#include "appConfig.hpp"
#include "sceneManager.hpp"
#include <filesystem>

static AppConfig instance;

AppConfig& AppConfig::get()
{
    return instance;
}

void AppConfig::initShaders()
{
    std::string fDeferedShader = std::filesystem::absolute("..\\..\\src\\shaders\\defered.frag").string();
    std::string vDeferedShader = std::filesystem::absolute("..\\..\\src\\shaders\\defered.vert").string();
    deferedShader = new Shader(vDeferedShader, fDeferedShader);
    SceneManager::addShader(deferedShader);
}