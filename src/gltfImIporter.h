#pragma once
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "glad/glad.h"
#include "tiny_gltf.h"
#include "primitive.h"   // Your Primitive struct/class (should include members like vao, vbo, ebo, etc.)
#include "shader.h"      // Your Shader class
#include <vector>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GLTFModel
{
public:
	Shader shader;
	std::vector<Primitive> primitives;
	std::string path;

	GLTFModel(std::string path, const Shader& shader) : path(path), shader(shader)
	{
		setup();
	}
	

	tinygltf::Model readGlb(const std::string &path)
	{		
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
		if (!warn.empty()) 
			printf("Warn: %s\n", warn.c_str());
		if (!err.empty()) 
			printf("Err: %s\n", err.c_str());
		if (!ret)
			printf("Failed to parse glTF\n");
		return model;
	}

	void processGLTFModel(tinygltf::Model &model)
	{
		for (auto &mesh : model.meshes)
		{
			for (auto &primitive : mesh.primitives)
			{	
				// --- Process POSITION attribute ---
				if (primitive.attributes.find("POSITION") == primitive.attributes.end())
				{
					std::cerr << "No POSITION attribute found in mesh " << mesh.name << std::endl;
					continue;
				}
				const auto &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
				const auto &posBufferView = model.bufferViews[posAccessor.bufferView];
				const auto &posBuffer = model.buffers[posBufferView.buffer];
				const float* pPosData = reinterpret_cast<const float*>(
					posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset);

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
				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
				{
					hasTexCoords = true;
					const auto &texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
					const auto &texBufferView = model.bufferViews[texAccessor.bufferView];
					const auto &texBuffer = model.buffers[texBufferView.buffer];
					const float* pTexData = reinterpret_cast<const float*>(
						texBuffer.data.data() + texBufferView.byteOffset + texAccessor.byteOffset);

					if (texAccessor.count != vertexCount)
					{
						std::cerr << "Mismatch in vertex count between POSITION and TEXCOORD_0 in mesh "
								  << mesh.name << std::endl;
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
				if (primitive.attributes.find("NORMAL") == primitive.attributes.end())
				{
					std::cerr << "No NORMAL attribute found in mesh " << mesh.name << std::endl;
					continue;
				}
				const auto &normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
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
				if (primitive.indices >= 0)
				{
					const auto &indexAccessor = model.accessors[primitive.indices];
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
				primitives.emplace_back(vao, vbo, ebo, shader, indexCount, glm::mat4(1.0f));
			}
		}
	}

	// Calls readGlb() and processGLTFModel() to load and set up the model.
	void setup()
	{
		auto model = readGlb(path);
		processGLTFModel(model);
	}

	// Set a uniform transform for all primitives in the model.
	void setTransform(glm::mat4 transform)
	{
		for (auto &primitive : primitives)
		{
			primitive.transform = transform;
		}
	}
};
