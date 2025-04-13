#include "sceneManager.hpp"

namespace SceneManager
{
	static Primitive* selectedPrimitive = nullptr;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;
	static std::vector<Mat> materials;
	static std::unordered_map<std::string, std::shared_ptr<Tex>> textureCache;
	static std::unordered_map<uint16_t, std::shared_ptr<Tex>> textureIndexing;

	void draw(Camera& camera,glm::mat4& lightSpaceMatrix, int32_t width, int32_t height) 
	{
		for (auto& primitive:primitives)
		{	
			primitive.draw(camera,lightSpaceMatrix, width, height);
			
		}
	}
	void draw(Camera& camera, glm::mat4& lightSpaceMatrix, int32_t width, int32_t height, uint32_t depthMap, float gamma) 
	{
		for (auto& primitive:primitives)
		{	
			primitive.draw(camera, lightSpaceMatrix, width, height, depthMap, gamma);
			
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
	}

	void addLights(const std::vector<Light> &lights)
	{
		for (auto& light: lights)
		{
			SceneManager::lights.push_back(std::move(light));
		}
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

	std::vector<Mat> &getMaterials()
	{
		return materials;
	}

	void addMaterials(const std::vector<Mat> &materials)
	{
		for (auto& material: materials)
		{
			SceneManager::materials.push_back(std::move(material));
		}
	}

	void addMaterial(Mat &material)
	{
		materials.push_back(std::move(material));
	}

	std::unordered_map<std::string, std::shared_ptr<Tex>> &getTextureCache()
	{
		return textureCache;
	}

	void addTextureToCache(const std::string &name, std::shared_ptr<Tex> texture)
	{
		textureCache[name] = texture;
	}

	std::unordered_map<uint16_t, std::shared_ptr<Tex>> getTextureIndexing()
	{
		return textureIndexing;
	}

	void addTextureIndex(uint16_t index, std::shared_ptr<Tex> texture)
	{
		textureIndexing[index] = texture;
	}

	void setShader(Shader &shader)
	{
		for (auto& primitive: primitives)
		{
			primitive.shader = shader;
		}
	}
};