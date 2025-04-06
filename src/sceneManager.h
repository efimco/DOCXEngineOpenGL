#pragma once
#include <vector>
#include <unordered_map>
#include "primitive.hpp"
#include "light.hpp"

namespace SceneManager
{
	static Primitive* selectedPrimitive;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;
	static std::vector<Mat> materials;
	static std::unordered_map<std::string, std::shared_ptr<Tex>> textureCache;
	static std::unordered_map<uint16_t, std::shared_ptr<Tex>> textureIndexing;

	static void draw(Camera& camera,glm::mat4& lightSpaceMatrix, int32_t width, int32_t height) 
	{
		for (auto& primitive:primitives)
		{	
			primitive.draw(camera,lightSpaceMatrix, width, height);
			
		}
	}
	static void draw(Camera& camera, glm::mat4& lightSpaceMatrix, int32_t width, int32_t height, uint32_t depthMap, float gamma) 
	{
		for (auto& primitive:primitives)
		{	
			primitive.draw(camera, lightSpaceMatrix, width, height, depthMap, gamma);
			
		}
	}

	static void addPrimitives(std::vector<Primitive>& Primitives)
	{
		for (auto& primitive : Primitives)
		{
			primitives.push_back(std::move(primitive)); 
		}
	}
	
	static void addLight(Light& light)
	{
		lights.push_back(std::move(light));
	}

	static void reloadShaders()
	{
		for (auto& primitive: primitives)
		{
			primitive.shader.reload();
			
		}
	}
	
	static void addMaterial(Mat& material)
	{
		materials.push_back(std::move(material));
	}
	static void setShader(Shader& shader)
	{
		for (auto& primitive: primitives)
		{
			primitive.shader = shader;
		}
	}
};