#pragma once
#include <vector>
#include "gltfImIporter.h"
#include "light.h"
namespace ObjectManager
{
	static Primitive* selectedPrimitive;
	static std::vector<Light> lights;
	static std::vector<Primitive> primitives;

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
	
};