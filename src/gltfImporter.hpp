#pragma once
#include <vector>
#include <string>
#include <map>
#include "shader.hpp"

class GLTFModel
{
	public:
		Shader shader;
		std::vector<Primitive> primitives;
		std::string path;
		std::map<uint32_t, Tex> textures;
		std::map<uint32_t, Mat> materials;


		GLTFModel(std::string path, const Shader& shader);
		
		~GLTFModel();

		tinygltf::Model readGlb(const std::string &path);


		void processGLTFModel(tinygltf::Model &model);

		void setup();

		void setTransform(glm::mat4 transform);
};
