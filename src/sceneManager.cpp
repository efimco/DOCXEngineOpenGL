#include "sceneManager.hpp"
#include <glad/gl.h>
#include <iostream>
#include <utility>
namespace SceneManager
{
	static Primitive *selectedPrimitive = nullptr;
	static std::vector<uint32_t> selectedPrimitives;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;
	static std::vector<Shader *> shaders;
	static std::unordered_map<uint32_t, std::shared_ptr<Mat>> materials;
	static std::unordered_map<std::string, std::shared_ptr<Tex>> textureCache;

	void addPrimitives(std::vector<Primitive> &&Primitives)
	{
		for (auto &primitive : Primitives)
		{
			SceneManager::primitives.push_back(std::move(primitive));
		}
	}

	std::vector<Primitive> &getPrimitives()
	{
		return primitives;
	}

	void selectPrimitive(int32_t vao, bool addToSelection)
	{
		auto found = std::find(selectedPrimitives.begin(), selectedPrimitives.end(), vao);
		if (vao == 0)
		{
			std::fill(selectedPrimitives.begin(), selectedPrimitives.end(), 0);
			selectedPrimitive = nullptr;
		}
		else
		{
			if (selectedPrimitives.size() <= vao)
			{
				selectedPrimitives.resize(vao + 1, 0);
			}

			if (addToSelection)
			{
				if (selectedPrimitives[vao] == 1)
				{
					selectedPrimitives[vao] = 0; // Deselect if already selected
				}
				else
				{
					selectedPrimitives[vao] = 1;
				}
			}
			else
			{
				std::fill(selectedPrimitives.begin(), selectedPrimitives.end(), 0);
				selectedPrimitives[vao] = 1;
			}
			selectedPrimitive = std::find_if(primitives.begin(), primitives.end(),
											 [vao](const Primitive &p)
											 { return p.vao == vao; })
									.operator->();
		}

		std::cout << "Selected primitives: ";
		for (size_t i = 0; i < selectedPrimitives.size(); i++)
		{
			if (selectedPrimitives[i])
				std::cout << i << " ";
		}
		std::cout << std::endl;
	}

	Primitive *getSelectedPrimitive()
	{
		return selectedPrimitive;
	}

	std::vector<uint32_t> getSelectedPrimitives()
	{
		return selectedPrimitives;
	}

	void deletePrimitive(uint32_t vao)
	{
		auto it = std::remove_if(primitives.begin(), primitives.end(),
								 [vao](const Primitive &p)
								 { return p.vao == vao; });
		primitives.erase(it, primitives.end());
		if (selectedPrimitive && selectedPrimitive->vao == vao)
		{
			selectedPrimitive = nullptr;
		}
	}

	void addShader(Shader *shader)
	{
		shaders.push_back(shader);
	}

	void addLight(Light &light)
	{
		lights.push_back(std::move(light));
	}

	std::vector<Light> &getLights()
	{
		return lights;
	}

	void reloadShaders()
	{
		for (auto &shader : shaders)
		{
			shader->reload();
		}
	}

	std::unordered_map<uint32_t, std::shared_ptr<Mat>> &getMaterials()
	{
		return materials;
	}

	std::shared_ptr<Mat> &getMaterial(uint32_t uid)
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

	std::shared_ptr<Tex> &getTexture(std::string name)
	{
		return textureCache[name];
	}

	void addTexture(const std::string &name, std::shared_ptr<Tex> texture)
	{
		textureCache[name] = texture;
	}
};