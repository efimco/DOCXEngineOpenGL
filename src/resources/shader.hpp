#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <string>


class Shader
{
	public:
		uint32_t shaderProgram;
		std::string vPath;
		std::string fPath;
		std::string cPath;
		Shader() = default;
		Shader(std::string vertexPath, std::string fragmentPath);
		Shader(std::string computePath);

		void use() const;

		void setBool(const std::string &name, bool value) const;
		void setInt(const std::string &name, int32_t value) const;
		void setUInt(const std::string &name, uint32_t value) const;
		void setIntArray(const std::string &name, uint32_t count, const int32_t *value) const;
		void setFloat(const std::string &name, float value) const;
		void setMat4(const std::string &name, glm::mat4 value) const;
		void setVec2(const std::string &name, float x, float y) const;
		void setVec3(const std::string &name, float x, float y, float z) const;
		void setVec3(const std::string &name, glm::vec3 value) const;

		void setVec4(const std::string &name, float x, float y, float z, float w) const;
		void setVec4(const std::string &name, glm::vec4 value) const;

		float getFLoat(const std::string &name) const;

		glm::vec4 getVec4(const std::string &name) const;
		glm::vec3 getVec3(const std::string &name) const;

		void reload();

	private:
		std::string vertexCode;
		std::string fragmentCode;
		std::string computeCode;

		std::string readShaderFromFile(std::string filePath);
};