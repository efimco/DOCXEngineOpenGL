#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

class Shader
{
	public:
		uint32_t shaderProgram;

		Shader(std::string vertexPath, std::string fragmentPath);
		
		void use();

		void setBool(const std::string &name, bool value) const;
		void setInt(const std::string &name, int32_t value) const;
		void setFloat(const std::string &name, float value) const;
		void setMat4(const std::string &name, glm::mat4 value) const;

	private:
		std::string vertexCode;
		std::string fragmentCode;
};