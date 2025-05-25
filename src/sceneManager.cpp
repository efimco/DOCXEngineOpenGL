#include "sceneManager.hpp"
#include <glad/gl.h>
#include <iostream>
#include <utility>
namespace SceneManager
{
	static Primitive* selectedPrimitive = nullptr;
	static std::vector<Primitive*> selectedPrimitives;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;
	static std::vector<Shader*> shaders;
	static std::unordered_map<uint32_t, std::shared_ptr<Mat>> materials;
	static std::unordered_map<std::string, std::shared_ptr<Tex>> textureCache;
	static uint32_t lightSSBO = 0;

	void addPrimitives(std::vector<Primitive>&& Primitives)
	{
		for (auto& primitive : Primitives)
		{
			SceneManager::primitives.push_back(std::move(primitive)); 
		}
	}

	std::vector<Primitive>& getPrimitives()
	{
		return primitives;
	}
	void selectPrimitive(uint32_t vao)
	{
		if (vao == 0)
		{
			selectedPrimitive = nullptr;
		} 
		for (auto& primitive : primitives)
		{
			if (primitive.vao == vao)
			{
				selectedPrimitive = &primitive;
				
				std::cout << "Selected primitive bounds: min(" 
					<< selectedPrimitive->boundingBox.first.x << ", "
					<< selectedPrimitive->boundingBox.first.y << ", "
					<< selectedPrimitive->boundingBox.first.z << ") max("
					<< selectedPrimitive->boundingBox.second.x << ", "
					<< selectedPrimitive->boundingBox.second.y << ", "
					<< selectedPrimitive->boundingBox.second.z << ")\n";
			}
		}
	}

	Primitive *getSelectedPrimitive()
	{
		return selectedPrimitive;
	}

	void deletePrimitive(uint32_t vao)
	{
		auto it = std::remove_if(primitives.begin(), primitives.end(), 
			[vao](const Primitive& p) { return p.vao == vao; });
		primitives.erase(it, primitives.end());
		if(selectedPrimitive && selectedPrimitive->vao == vao)
		{
			selectedPrimitive = nullptr;
		}
	}

	void addShader(Shader* shader)
	{
		shaders.push_back(shader);
	}

	void addLight(Light& light)
	{
		lights.push_back(std::move(light));
	}

	std::vector<Light>& getLights()
	{
		return lights;
	}

	void reloadShaders()
	{
		for (auto& shader: shaders)
		{
			shader->reload();
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
};