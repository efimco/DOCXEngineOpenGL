#pragma once
#include <vector>
#include "gltfImIporter.h"

class ObjectManager
{
	public:
		static std::vector<Primitive> primitives;
		static Primitive* selectedPrimitive;
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
		
		static void reloadShaders()
		{
			for (auto& primitive: primitives)
			{
				primitive.shader.reload();
			}
		}

};

std::vector<Primitive> ObjectManager::primitives; 
Primitive* ObjectManager::selectedPrimitive = nullptr;


