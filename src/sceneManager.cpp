#include "sceneManager.hpp"
#include <glad/glad.h>
#include <iostream>

namespace SceneManager
{
	static Primitive* selectedPrimitive = nullptr;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;
	static std::unordered_map<uint32_t, std::shared_ptr<Mat>> materials;
	static std::unordered_map<std::string, std::shared_ptr<Tex>> textureCache;
	static uint32_t lightSSBO = 0;

	void draw(Camera& camera, int32_t width, int32_t height, uint32_t depthMap, float gamma, uint32_t cubemapID)
	{
		for (auto& primitive:primitives)
		{	
			primitive.draw(camera, width, height, depthMap, gamma, cubemapID);
		}
	}

	void addPrimitives(std::vector<Primitive>&& Primitives)
	{
		for (auto& primitive : Primitives)
		{
			SceneManager::primitives.push_back(std::move(primitive)); 
		}
	}

	void addPrimitive(Primitive&& primitive)
	{
		SceneManager::primitives.push_back(std::move(primitive)); 
	}

	std::vector<Primitive>& getPrimitives()
	{
		return primitives;
	}

	Primitive *getSelectedPrimitive()
	{
		return selectedPrimitive;
	}

	void setSelectedPrimitive(Primitive* primitive)
	{
		selectedPrimitive = primitive;
	}

	void addLight(Light& light)
	{
		lights.push_back(std::move(light));
		glNamedBufferData(lightSSBO, lights.size() * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
	}

	void updateLights()
	{
			glNamedBufferSubData(lightSSBO, 0, lights.size() * sizeof(Light), lights.data());
	}

	void checkLightBuffer() 
	{
		GLint bufferSize = 0;
		glGetNamedBufferParameteriv(lightSSBO, GL_BUFFER_SIZE, &bufferSize);
		std::cout << "Light buffer size: " << bufferSize << " bytes" << std::endl;
		std::cout << "Lights count: " << SceneManager::getLights().size() << std::endl;
		std::cout << "Light size: " << sizeof(Light) << std::endl;
	}

	void createLightsSSBO()
	{
		glCreateBuffers(1, &SceneManager::getLightsSSBO());
		glNamedBufferData(SceneManager::getLightsSSBO(), SceneManager::getLights().size() * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SceneManager::getLightsSSBO());
	}

	uint32_t& getLightsSSBO()
	{
		return lightSSBO;
	}

	std::vector<Light>& getLights()
	{
		return lights;
	}

	void reloadShaders()
	{
		for (auto& primitive: primitives)
		{
			primitive.shader.reload();
			
		}
	}

	std::unordered_map<uint32_t, std::shared_ptr<Mat>>& getMaterials()
	{
		return materials;
	}

	std::shared_ptr<Mat>& getMaterial(uint32_t uid)
	{
		return materials[uid];
	}

	void addMaterial(std::shared_ptr<Mat> &material, uint32_t uid)
	{
		SceneManager::materials[uid] = material;
	}

	std::unordered_map<std::string, std::shared_ptr<Tex>> &getTextureCache()
	{
		return textureCache;
	}

	std::shared_ptr<Tex>& getTexture(std::string name)
	{
		return textureCache[name];
	}

	void addTexture(const std::string &name, std::shared_ptr<Tex> texture)
	{
		textureCache[name] = texture;
	}

	void setShader(Shader &shader)
	{
		for (auto& primitive: primitives)
		{
			primitive.shader = shader;
		}
	}
};