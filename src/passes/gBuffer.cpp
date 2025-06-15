#include "gBuffer.hpp"
#include <string>
#include <filesystem>

GBuffer::GBuffer()
{

	std::string fShaderPath = std::filesystem::absolute("..\\..\\src\\shaders\\picking.frag").string();
}



void GBuffer::initTextures()
{

}