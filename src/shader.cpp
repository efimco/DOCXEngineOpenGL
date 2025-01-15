#include "shader.h"


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
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	return shaderCode;
}


Shader::Shader(std::string vShaderPath, std::string fShaderPath)
{
	std::string vertexCode = readShaderFromFile(vShaderPath);
	std::string fragmentCode = readShaderFromFile(fShaderPath);

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
		std::cout << "ERROR::SHADER_PROGRAM::LINKING::FAILED\n" << infoLog << '\n' << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader); 
}

void Shader::use()
{
	glUseProgram(shaderProgram);
}

void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(shaderProgram,name.c_str()),(int)value);
}

void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()),value);
}

void Shader::setInt(const std::string &name, int32_t value) const
{
	glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setMat4(const std::string &name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram,name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}