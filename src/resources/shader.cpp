#pragma once
#include "shader.hpp"
#include <filesystem>
#include <fstream>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <regex>
#include <sstream>


Shader::Shader(std::string vertexPath, std::string fragmentPath)
{
    vPath = vertexPath;
    fPath = fragmentPath;
    vertexCode = readShaderFromFile(vertexPath);
    fragmentCode = readShaderFromFile(fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    uint32_t vertexShader, fragmentShader;
    int success;
    char infoLog[512];

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);

    // validate vertex shader
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // create fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);

    // create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // validate program
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {

        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM::LINKING::FAILED\n" << fragmentPath << '\n' << infoLog << '\n' << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(std::string computePath)
{
    cPath = computePath;
    computeCode = readShaderFromFile(computePath);

    const char* cShaderCode = computeCode.c_str();

    uint32_t computeShader;
    int success;
    char infoLog[512];

    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &cShaderCode, NULL);
    glCompileShader(computeShader);

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(computeShader, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, computeShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM::LINKING::FAILED\n" << computePath << '\n' << infoLog << '\n' << std::endl;
    }

    glDeleteShader(computeShader);
}

void Shader::use() const { glUseProgram(shaderProgram); }

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), (int)value);
}

void Shader::setUInt(const std::string& name, uint32_t value) const
{
    glUniform1ui(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int32_t value) const
{
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setIntArray(const std::string& name, uint32_t count, const int32_t* value) const
{
    glUniform1iv(glGetUniformLocation(shaderProgram, name.c_str()), count, value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}
void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(shaderProgram, name.c_str()), x, y);
}
void Shader::setVec2(const std::string& name, glm::vec2 value) const
{
    glUniform2fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}
void Shader::setVec3(const std::string& name, glm::vec3 value) const
{
    glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
}
void Shader::setVec4(const std::string& name, glm::vec4 value) const
{
    glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, glm::value_ptr(value));
}

float Shader::getFLoat(const std::string& name) const
{
    float value;
    glGetUniformfv(shaderProgram, glGetUniformLocation(shaderProgram, name.c_str()), &value);
    return value;
}

glm::vec4 Shader::getVec4(const std::string& name) const
{
    float value[4];
    glGetUniformfv(shaderProgram, glGetUniformLocation(shaderProgram, name.c_str()), value);
    return glm::vec4(value[0], value[1], value[2], value[3]);
}
glm::vec3 Shader::getVec3(const std::string& name) const
{
    float value[3];
    glGetUniformfv(shaderProgram, glGetUniformLocation(shaderProgram, name.c_str()), value);
    return glm::vec3(value[0], value[1], value[2]);
}

void Shader::reload()
{
    glDeleteProgram(shaderProgram);

    *this = Shader(vPath, fPath);
}

std::string Shader::readShaderFromFile(std::string filePath)
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
    }
    catch (const std::ifstream::failure e)
    {
        std::cout << std::filesystem::absolute(filePath) << std::endl;
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    std::string processedShader;
    std::string line;
    std::filesystem::path parentPath = std::filesystem::absolute("..\\..\\src\\shaders").string();
    while (std::getline(shaderStream, line))
    {
        std::regex includeRegex(R"(^\s*#include\s+[\"<](.+)[\">])");
        std::smatch matches;
        if (std::regex_search(line, matches, includeRegex))
        {
            std::string includeFileName = matches[1].str();
            std::filesystem::path includePath = parentPath / includeFileName;
            std::ifstream includeFile(includePath);
            if (includeFile)
            {
                std::string includeContent;
                includeContent = readShaderFromFile(includePath.string());
                processedShader += includeContent;
            }
            else
                std::cout << "ERROR::SHADER::INCLUDE_FILE_NOT_SUCCESFULLY_READ: " << includePath << std::endl;
            continue;
        }
        processedShader += line + "\n";
    }
    shaderCode = processedShader;

    return shaderCode;
}