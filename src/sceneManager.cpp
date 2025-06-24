#include "sceneManager.hpp"
#include <glad/gl.h>
#include <iostream>
#include <utility>
namespace SceneManager
{
	static Primitive* selectedPrimitive = nullptr;
	static std::vector<SceneGraph::Model*> models;
	static std::vector<uint32_t> selectedPrimitives;
	static std::vector<Light*> lights;
	static std::vector<Primitive*> primitives;
	static std::vector<Shader*> shaders;
	static std::unordered_map<uint32_t, std::shared_ptr<Mat>> materials;
	static std::unordered_map<std::string, std::shared_ptr<Tex>> textureCache;

	void addPrimitive(Primitive* primitive)
	{
		SceneManager::primitives.push_back(std::move(primitive));
	}

	void removePrimitive(Primitive* primitive)
	{
		auto it = std::remove_if(primitives.begin(), primitives.end(), [primitive](const Primitive* p)
			{
				return p->vao == primitive->vao;
			});

		if (it != primitives.end())
		{
			primitives.erase(it, primitives.end());
			if (selectedPrimitive && selectedPrimitive->vao == primitive->vao)
			{
				selectedPrimitive = nullptr;
			}
		}
	}

	std::vector<Primitive*> getPrimitives()
	{
		return primitives;
	}

	void selectPrimitive(int32_t vao, bool addToSelection)
	{
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
				// Toggle selection state for the given vao
				bool isSelected = selectedPrimitives[vao] == 1;
				selectedPrimitives[vao] = isSelected ? 0 : 1;
			}
			else
			{
				std::fill(selectedPrimitives.begin(), selectedPrimitives.end(), 0);
				selectedPrimitives[vao] = 1;
			}
			auto it = std::find_if(primitives.begin(), primitives.end(), [vao](Primitive* p) { return p->vao == vao; });
			selectedPrimitive = (it != primitives.end()) ? *it : nullptr;
		}
		std::cout << "Selected primitives: ";
		for (size_t i = 0; i < selectedPrimitives.size(); i++)
		{
			if (selectedPrimitives[i])
				std::cout << i << " ";
		}
	}

	Primitive* getSelectedPrimitive()
	{
		return selectedPrimitive;
	}

	std::vector<uint32_t> getSelectedPrimitives()
	{
		return selectedPrimitives;
	}

	void addModel(SceneGraph::Model* model)
	{
		models.push_back(std::move(model));
	}

	std::vector<SceneGraph::Model*> getModels()
	{
		return models;
	}

	void addLight(Light* light)
	{
		lights.push_back(std::move(light));
	}

	void removeLight(Light* light)
	{
		auto it = std::remove_if(lights.begin(), lights.end(), [light](const Light* l)
			{
				return l->name == light->name;
			});

		if (it != lights.end())
		{
			lights.erase(it, lights.end());
		};
	}

	void addShader(Shader* shader)
	{
		shaders.push_back(shader);
	}

	void reloadShaders()
	{
		for (auto& shader : shaders)
		{
			shader->reload();
		}
	}

	std::unordered_map<uint32_t, std::shared_ptr<Mat>>& getMaterials() { return materials; }

	std::shared_ptr<Mat>& getMaterial(uint32_t uid) { return materials[uid]; }

	void addMaterial(std::shared_ptr<Mat>& material, uint32_t uid) { SceneManager::materials[uid] = material; }

	std::unordered_map<std::string, std::shared_ptr<Tex>>& getTextureCache() { return textureCache; }

	std::shared_ptr<Tex>& getTexture(std::string name) { return textureCache[name]; }

	void addTexture(const std::string& name, std::shared_ptr<Tex> texture) { textureCache[name] = texture; }
}; // namespace SceneManager