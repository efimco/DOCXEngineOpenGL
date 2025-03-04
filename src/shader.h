#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

class Shader
{
	public:
		uint32_t shaderProgram;
		std::string vPath;
		std::string fPath;
		Shader(std::string vertexPath, std::string fragmentPath)
		{
			vPath = vertexPath;
			fPath = fragmentPath;
			vertexCode = readShaderFromFile(vertexPath);
			fragmentCode = readShaderFromFile(fragmentPath);

			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();

			uint32_t vertexShader,fragmentShader;
			int success;
			char infoLog[512];

			vertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertexShader,1,&vShaderCode,NULL);
			glCompileShader(vertexShader);

			//validate vertex shader
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS,&success);
			if(!success)
			{
				glGetShaderInfoLog(vertexShader,sizeof(infoLog),NULL,infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}

			//create fragment shader
			fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragmentShader,1,&fShaderCode,NULL);
			glCompileShader(fragmentShader);


			//create shader program
			shaderProgram = glCreateProgram();
			glAttachShader(shaderProgram,vertexShader);
			glAttachShader(shaderProgram,fragmentShader);
			glLinkProgram(shaderProgram);

			//validate program
			glGetProgramiv(shaderProgram,GL_LINK_STATUS,&success);
			if(!success)
			{
				
				glGetProgramInfoLog(shaderProgram,sizeof(infoLog),NULL, infoLog);
				std::cout << "ERROR::SHADER_PROGRAM::LINKING::FAILED\n"  << fragmentPath << '\n' << infoLog << '\n' << std::endl;
			}
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader); 
		}
		
		void use() const
		{
			glUseProgram(shaderProgram);
		}

		void setBool(const std::string &name, bool value) const
		{

			glUniform1i(glGetUniformLocation(shaderProgram,name.c_str()),(int)value);
		}
		void setInt(const std::string &name, int32_t value) const
		{

			glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()),value);
		}
		void setFloat(const std::string &name, float value) const
		{

			glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
		}
		void setMat4(const std::string &name, glm::mat4 value) const
		{

			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
		}
		void setVec3(const std::string &name, float x, float y, float z) const
		{

			glUniform3f(glGetUniformLocation(shaderProgram,name.c_str()), x, y, z );
		}
		void setVec3(const std::string &name, glm::vec3 value) const
		{
	
			glUniform3fv(glGetUniformLocation(shaderProgram,name.c_str()),1, glm::value_ptr(value));
		}

		void setVec4(const std::string &name, float x, float y, float z, float w) const
		{
		
			glUniform4f(glGetUniformLocation(shaderProgram,name.c_str()), x, y, z, w );
		}
		void setVec4(const std::string &name, glm::vec4 value) const
		{
		
			glUniform4fv(glGetUniformLocation(shaderProgram,name.c_str()),1, glm::value_ptr(value));
		}

		float getFLoat(const std::string &name) const
		{
	
			float value;
			glGetUniformfv(shaderProgram, glGetUniformLocation(shaderProgram, name.c_str()), &value);
			return value;
		}

		glm::vec4 getVec4(const std::string &name) const
		{

			float value[4];
			glGetUniformfv(shaderProgram, glGetUniformLocation(shaderProgram, name.c_str()), value);
			return glm::vec4(value[0], value[1], value[2], value[3]);
		}
		glm::vec3 getVec3(const std::string &name) const
		{

			float value[3];
			glGetUniformfv(shaderProgram, glGetUniformLocation(shaderProgram, name.c_str()), value);
			return glm::vec3(value[0], value[1], value[2]);
		}

		void reload() 
		{
			glDeleteProgram(shaderProgram);

			*this = Shader(vPath, fPath);
		}

	private:
		std::string vertexCode;
		std::string fragmentCode;

		std::string readShaderFromFile(std::string filePath)
		{	
			std::string shaderCode;
			std::ifstream shaderFile;
			std::stringstream shaderStream;


			shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{
				shaderFile.open(filePath);

				shaderStream << shaderFile.rdbuf();
				shaderFile.close();
				shaderCode = shaderStream.str();
			}
			catch(const std::ifstream::failure e)
			{
				
				std::cout << std::filesystem::absolute(filePath) << std::endl;
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
			return shaderCode;
		}
};