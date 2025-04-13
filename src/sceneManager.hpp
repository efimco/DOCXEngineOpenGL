#pragma once
#include <vector>
#include <unordered_map>
#include "primitive.hpp"
#include "light.hpp"

namespace SceneManager
{
	void draw(Camera& camera,glm::mat4& lightSpaceMatrix, int32_t width, int32_t height);
	void draw(Camera& camera, glm::mat4& lightSpaceMatrix, int32_t width, int32_t height, uint32_t depthMap, float gamma);
	
	void addPrimitives(std::vector<Primitive>&& Primitives);
	std::vector<Primitive>& getPrimitives();
	void addPrimitive(Primitive&& primitive);

	Primitive* getSelectedPrimitive();
	void setSelectedPrimitive(Primitive* primitive);

	std::vector<Mat>& getMaterials();
	void addMaterials(const std::vector<Mat>& materials);
	void addMaterial(const Mat& material);

	std::unordered_map<std::string, std::shared_ptr<Tex>>& getTextureCache();
	void addTextureToCache(const std::string& name, std::shared_ptr<Tex> texture);

	std::unordered_map<uint16_t, std::shared_ptr<Tex>> getTextureIndexing();
	void addTextureIndex(uint16_t index, std::shared_ptr<Tex> texture);

	void addLight(Light& light);
	void addLights(std::vector<Light>& lights);
	std::vector<Light>& getLights();

	void reloadShaders();
	void setShader(Shader& shader);
};