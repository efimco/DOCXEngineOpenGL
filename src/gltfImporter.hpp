#pragma once
#include <vector>
#include <string>
#include <map>
#include "shader.hpp"
#include "tiny_gltf.h"
#include "primitive.hpp"

class GLTFModel
{
public:
	Shader shader;
	std::vector<Primitive> primitives;
	std::string path;
	std::map<uint32_t, std::shared_ptr<Tex>> texturesIndex;
	std::map<uint32_t, std::shared_ptr<Mat>> materialsIndex;

	GLTFModel(std::string path, const Shader& shader);
	~GLTFModel();

	tinygltf::Model readGlb(const std::string &path);
	void processGLTFModel(tinygltf::Model &model);
	void processTextures(tinygltf::Model &model);
	void processMaterials(tinygltf::Model &model);
	const std::vector<float> processPosAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model);
	const std::vector<float> processTexCoordAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model);
	const std::vector<float> processNormalAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model);
	const std::vector<float> processTangentNormalAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model);
	const std::vector<uint32_t> processIndexAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model);

	void setTransform(glm::mat4 transform);
};
