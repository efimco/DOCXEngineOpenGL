#include <string>
#include <iostream>
#include "glad/glad.h"
#include "shader.hpp"
#include "tiny_gltf.h"
#include "sceneManager.h"
#include "gltfImporter.hpp"

GLTFModel::GLTFModel(std::string path, const Shader& shader) : path(path), shader(shader)
{
	setup();
}

GLTFModel::~GLTFModel(){};

tinygltf::Model GLTFModel::readGlb(const std::string &path)
{		
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	printf("Loading...\n");
	bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
	// bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
	if (!warn.empty()) 
		printf("Warn: %s\n", warn.c_str());
	if (!err.empty()) 
		printf("Err: %s\n", err.c_str());
	if (!ret)
		printf("Failed to parse glTF\n");
	printf("Loaded %s\n", path.c_str());
	return model;
}


void GLTFModel::processGLTFModel(tinygltf::Model &model)
{

	for (int i = 0; i < model.images.size(); i++)
	{
		// std::string path = std::filesystem::absolute("..\\..\\res\\GltfModels\\" + model.images[i].name + ".png").string();
		std::string path = model.images[i].name;
		if (SceneManager::textureCache.find(path) == SceneManager::textureCache.end()) 
		{
			SceneManager::textureCache[path] = std::make_shared<Tex>(model.images[i], "texture");
		}
		SceneManager::textureIndexing[i] = SceneManager::textureCache[path];
	} 
	
	for (int i = 0; i < model.materials.size(); i++)
	{
		Mat mat;
		if (model.materials[i].pbrMetallicRoughness.baseColorTexture.index != -1)
		{
			SceneManager::textureIndexing[model.materials[i].pbrMetallicRoughness.baseColorTexture.index] -> type = "tDiffuse";
			mat.diffuse = SceneManager::textureIndexing[model.materials[i].pbrMetallicRoughness.baseColorTexture.index];
		}
			
		if (model.materials[i].pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
		{
			SceneManager::textureIndexing[model.materials[i].pbrMetallicRoughness.metallicRoughnessTexture.index] -> type = "tSpecular";	
			mat.specular = SceneManager::textureIndexing[model.materials[i].pbrMetallicRoughness.metallicRoughnessTexture.index];

		}
		if (model.materials[i].normalTexture.index != -1)
		{
			SceneManager::textureIndexing[model.materials[i].normalTexture.index] -> type = "tNormal";	
			mat.normal = SceneManager::textureIndexing[model.materials[i].normalTexture.index];

		}
		materials[i] = mat;
	}

	for (int i =0; i< model.meshes.size(); i++) 
	{
		for (int j = 0; j < model.meshes[i].primitives.size(); j++)
		{	
			// --- Process POSITION attribute ---
			if (model.meshes[i].primitives[j].attributes.find("POSITION") == model.meshes[i].primitives[j].attributes.end())
			{
				std::cerr << "No POSITION attribute found in mesh " << model.meshes[i].name << std::endl;
				continue;
			}
			const auto &posAccessor = model.accessors[model.meshes[i].primitives[j].attributes.at("POSITION")];
			const auto &posBufferView = model.bufferViews[posAccessor.bufferView];
			const auto &posBuffer = model.buffers[posBufferView.buffer];
			const float* pPosData = reinterpret_cast<const float*>(posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset);

			size_t vertexCount = posAccessor.count;
			int posComponents = (posAccessor.type == TINYGLTF_TYPE_VEC3) ? 3 : 0;
			std::vector<float> posData;
			posData.reserve(vertexCount * posComponents);
			for (size_t i = 0; i < vertexCount; i++)
			{
				for (int j = 0; j < posComponents; j++)
				{
					posData.push_back(pPosData[i * posComponents + j]);
				}
			}

			// --- Process TEXCOORD_0 attribute if available ---
			bool hasTexCoords = false;
			std::vector<float> texCoordData;
			int texComponents = 0;
			if (model.meshes[i].primitives[j].attributes.find("TEXCOORD_0") != model.meshes[i].primitives[j].attributes.end())
			{
				hasTexCoords = true;
				const auto &texAccessor = model.accessors[model.meshes[i].primitives[j].attributes.at("TEXCOORD_0")];
				const auto &texBufferView = model.bufferViews[texAccessor.bufferView];
				const auto &texBuffer = model.buffers[texBufferView.buffer];
				const float* pTexData = reinterpret_cast<const float*>(
					texBuffer.data.data() + texBufferView.byteOffset + texAccessor.byteOffset);

				if (texAccessor.count != vertexCount)
				{
					std::cerr << "Mismatch in vertex count between POSITION and TEXCOORD_0 in mesh "
							<< model.meshes[i].name << std::endl;
					continue;
				}
				texComponents = (texAccessor.type == TINYGLTF_TYPE_VEC2) ? 2 : 0;
				texCoordData.reserve(vertexCount * texComponents);
				for (size_t i = 0; i < vertexCount; i++)
				{
					for (int j = 0; j < texComponents; j++)
					{
						texCoordData.push_back(pTexData[i * texComponents + j]);
					}
				}
			}
			std::vector<float> normalData;
			// --- Process Normal attribute ---
			if (model.meshes[i].primitives[j].attributes.find("NORMAL") == model.meshes[i].primitives[j].attributes.end())
			{
				std::cerr << "No NORMAL attribute found in mesh " << model.meshes[i].name << std::endl;
				continue;
			}
			const auto &normalAccessor = model.accessors[model.meshes[i].primitives[j].attributes.at("NORMAL")];
			const auto &normalBufferView = model.bufferViews[normalAccessor.bufferView];
			const auto &normalBuffer = model.buffers[normalBufferView.buffer];
			const float* pNormalData = reinterpret_cast<const float*>(
				normalBuffer.data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset);

			int normalComponents = (normalAccessor.type == TINYGLTF_TYPE_VEC3) ? 3 : 0;
			normalData.reserve(normalAccessor.count * normalComponents);
			for (size_t i = 0; i < normalAccessor.count; i++)
			{
				for (int j = 0; j < normalComponents; j++)
				{
					normalData.push_back(pNormalData[i * normalComponents + j]);
				}
			}

			// --- Process indices if available ---
			size_t indexCount = 0;
			std::vector<uint32_t> indexData;
			if (model.meshes[i].primitives[j].indices >= 0)
			{
				const auto &indexAccessor = model.accessors[model.meshes[i].primitives[j].indices];
				const auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
				const auto &indexBuffer = model.buffers[indexBufferView.buffer];

				const void *pIndexData = indexBuffer.data.data() +
										indexBufferView.byteOffset + indexAccessor.byteOffset;
				indexCount = indexAccessor.count;
				if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					const uint16_t *indices = reinterpret_cast<const uint16_t *>(pIndexData);
					indexData.assign(indices, indices + indexCount);
				} 
				else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) 
				{
					const uint32_t *indices = reinterpret_cast<const uint32_t *>(pIndexData);
					indexData.assign(indices, indices + indexCount);
				}
				else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) 
				{
					const uint8_t *indices = reinterpret_cast<const uint8_t *>(pIndexData);
					indexData.assign(indices, indices + indexCount);
				}
			}

			// --- Create OpenGL buffers (VAO, VBO, and optionally EBO) ---
			uint32_t vao = 0, vbo = 0, ebo = 0;
			glCreateVertexArrays(1, &vao);
			
			if (hasTexCoords)
			{
				// Interleave positions and texture coordinates.
				int strideComponents = posComponents + texComponents + normalComponents; // e.g., 3 + 2 + 3 = 8
				std::vector<float> interleavedData;
				interleavedData.resize(vertexCount * strideComponents);
				for (size_t i = 0; i < vertexCount; i++)
				{
					// Copy position data.
					for (int j = 0; j < posComponents; j++)
					{
						interleavedData[i * strideComponents + j] = posData[i * posComponents + j];
					}
					// Copy texcoord data.
					for (int j = 0; j < texComponents; j++)
					{
						interleavedData[i * strideComponents + posComponents + j] = texCoordData[i * texComponents + j];
					}
					// Copy normal data.
					for (int j = 0; j < normalComponents; j++)
					{
						interleavedData[i * strideComponents + posComponents + texComponents + j] = normalData[i * normalComponents + j];
					}
				}
				glCreateBuffers(1, &vbo);
				glNamedBufferData(vbo, interleavedData.size() * sizeof(float),
									interleavedData.data(), GL_STATIC_DRAW);
				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * strideComponents);

				// Set attribute 0: position (first posComponents floats).
				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayAttribFormat(vao, 0, posComponents, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayAttribBinding(vao, 0, 0);

				// Set attribute 1: texture coordinates (next texComponents floats).
				glEnableVertexArrayAttrib(vao, 1);
				glVertexArrayAttribFormat(vao, 1, texComponents, GL_FLOAT, GL_FALSE, sizeof(float) * posComponents);
				glVertexArrayAttribBinding(vao, 1, 0);

				// Set attribute 2: normal coordinates (next normalComponents floats).
				glEnableVertexArrayAttrib(vao, 2);
				glVertexArrayAttribFormat(vao, 2, normalComponents, GL_FLOAT, GL_FALSE, sizeof(float) * (posComponents + texComponents));
				glVertexArrayAttribBinding(vao, 2, 0);
			}
			else
			{
				// Only position data is available.
				glCreateBuffers(1, &vbo);
				glNamedBufferData(vbo, posData.size() * sizeof(float),
									posData.data(), GL_STATIC_DRAW);
				glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * posComponents);
				glEnableVertexArrayAttrib(vao, 0);
				glVertexArrayAttribFormat(vao, 0, posComponents, GL_FLOAT, GL_FALSE, 0);
				glVertexArrayAttribBinding(vao, 0, 0);
			}

			if (!indexData.empty())
			{
				glCreateBuffers(1, &ebo);
				glNamedBufferData(ebo, indexData.size() * sizeof(uint32_t),
									indexData.data(), GL_STATIC_DRAW);
				glVertexArrayElementBuffer(vao, ebo);
			}
			glBindVertexArray(0);
			glm::mat4 translation(1.0f);
			if (model.nodes[i].translation.size() != 0)
				translation[3] = glm::vec4(model.nodes[i].translation[0], model.nodes[i].translation[1], model.nodes[i].translation[2], 1.0f);
			Primitive prim(vao, vbo, ebo,
				shader, indexCount,
				translation,
				materials[model.meshes[i].primitives[j].material]);
			primitives.push_back(std::move(prim));
		}
	}
}

void GLTFModel::setup()
{
	auto model = readGlb(path);
	processGLTFModel(model);
}

void GLTFModel::setTransform(glm::mat4 transform)
{
	for (auto &primitive : primitives)
	{
		primitive.transform += transform;
	}
}

