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

	std::unordered_map<uint32_t, std::shared_ptr<Mat>>& getMaterials();
	std::shared_ptr<Mat>& getMaterial(uint32_t uid);
	void addMaterial(std::shared_ptr<Mat> &material, uint32_t uid);

	std::unordered_map<std::string, std::shared_ptr<Tex>>& getTextureCache();
	std::shared_ptr<Tex>& getTexture(std::string name);
	void addTexture(const std::string& name, std::shared_ptr<Tex> texture);

	void addLight(Light& light);
	void updateLights();
	std::vector<Light>& getLights();
	void createLightsSSBO();
	uint32_t& getLightsSSBO();
	void checkLightBuffer();

	void reloadShaders();
	void setShader(Shader& shader);
	};