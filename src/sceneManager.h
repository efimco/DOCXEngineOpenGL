#pragma once
#include <vector>
#include "primitive.h"
#include "light.h"
namespace SceneManager
{
	static Primitive* selectedPrimitive;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;
	static std::vector<Mat> materials;

	static void draw(Camera& camera, int32_t width, int32_t height) 
	{
		for (auto& primitive:primitives)
		{	
			primitive.draw(camera, width, height);
			
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
};