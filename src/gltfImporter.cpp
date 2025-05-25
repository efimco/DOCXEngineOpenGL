#include <string>
#include <iostream>
#include <iterator>
#include "glad/gl.h"
#include "sceneManager.hpp"
#include "gltfImporter.hpp"


GLTFModel::GLTFModel(std::string path) : path(path)
{
	auto model = readGlb(path);
	processTextures(model);
	processMaterials(model);
	processGLTFModel(model);

}

GLTFModel::~GLTFModel()
{
	std::cout << "GLTF Importer destructor called!\n" << std::endl;
}

tinygltf::Model GLTFModel::readGlb(const std::string &path)
{		
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	printf("Loading...%s\n ",path.c_str());
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
	for (const auto& mesh: model.meshes) 
	{
		for (const auto& primitive: mesh.primitives)
		{	
			std::vector<float> posBuffer = processPosAttrib(primitive, mesh, model);
			std::vector<float> texBuffer = processTexCoordAttrib(primitive, mesh, model);
			std::vector<float> normalBuffer = processNormalAttrib(primitive, mesh, model);
			std::vector<float> tangentNormalBuffer = processTangentNormalAttrib(primitive, mesh, model);
			std::pair<glm::vec3, glm::vec3> boundingBox = computeBoundingBox(primitive, model);
			std::vector<uint32_t>indexBuffer = processIndexAttrib(primitive, mesh, model);
			if (posBuffer.empty() || texBuffer.empty() || normalBuffer.empty() || tangentNormalBuffer.empty())
			{
				std::cerr << "Failed to process attributes for mesh " << mesh.name << std::endl;
				continue;
			}
			if (indexBuffer.empty())
			{
				std::cerr << "Failed to process indices for mesh " << mesh.name << std::endl;
				continue;
			}
			uint32_t vao;
			uint32_t vbo;
			uint32_t ebo;

			glCreateVertexArrays(1, &vao);
			glCreateBuffers(1, &vbo);

			size_t posSize = posBuffer.size() * sizeof(float);
			size_t texSize = texBuffer.size() * sizeof(float);
			size_t normalSize = normalBuffer.size() * sizeof(float);
			size_t tangentNormalSize = tangentNormalBuffer.size() * sizeof(float);
			size_t bufferSize = posSize + texSize + normalSize +tangentNormalSize ;

			size_t indexSize = indexBuffer.size() * sizeof(uint32_t);

			// std::cout << "Pos buffer size: " << posSize << std::endl;
			// std::cout << "Tex buffer size: " << texSize << std::endl;
			// std::cout << "Norm buffer size: " << normalSize << std::endl;
			// std::cout << "TangNorm buffer size: " << tangentNormalSize << std::endl;
			// std::cout << "Index buffer size: " << indexSize << std::endl;
			// std::cout << "Final buffer size: " << bufferSize << std::endl;
			// std::cout << std::endl;


			glCreateBuffers(1, &ebo);
			glNamedBufferData(ebo, indexBuffer.size() * sizeof(uint32_t),indexBuffer.data(), GL_STATIC_DRAW);
			glVertexArrayElementBuffer(vao, ebo);


			glNamedBufferData(vbo, bufferSize, NULL, GL_STATIC_DRAW);
			glNamedBufferSubData(vbo, 0, posSize, posBuffer.data());
			glNamedBufferSubData(vbo, posSize, texSize, texBuffer.data());
			glNamedBufferSubData(vbo, posSize + texSize, normalSize, normalBuffer.data());
			glNamedBufferSubData(vbo, posSize + texSize+ normalSize, tangentNormalSize, tangentNormalBuffer.data());

			glVertexArrayVertexBuffer(vao, 0, vbo, 0, 3 * sizeof(float));
			glVertexArrayVertexBuffer(vao, 1, vbo, posSize, 2 * sizeof(float));
			glVertexArrayVertexBuffer(vao, 2, vbo, posSize + texSize, 3 * sizeof(float));
			glVertexArrayVertexBuffer(vao, 3, vbo, posSize + texSize + normalSize, 4 * sizeof(float));

			glEnableVertexArrayAttrib(vao, 0);
			glEnableVertexArrayAttrib(vao, 1);
			glEnableVertexArrayAttrib(vao, 2);
			glEnableVertexArrayAttrib(vao, 3);

			glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribFormat(vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
			glVertexArrayAttribFormat(vao, 3, 4, GL_FLOAT, GL_FALSE, 0);

			glVertexArrayAttribBinding(vao, 0, 0);
			glVertexArrayAttribBinding(vao, 1, 1);
			glVertexArrayAttribBinding(vao, 2, 2);
			glVertexArrayAttribBinding(vao, 3, 3);

			glm::mat4 translation(1.0f);
			size_t meshIndex = &mesh - &model.meshes[0];
			if (model.nodes[meshIndex].translation.size() != 0)
				translation[3] = glm::vec4(model.nodes[meshIndex].translation[0], model.nodes[meshIndex].translation[1], model.nodes[meshIndex].translation[2], 1.0f);
		
				assert(indexBuffer.size() == model.accessors[primitive.indices].count);
			size_t indexCount = indexBuffer.size();
			Primitive prim(vao, vbo, ebo,
				indexCount,
				translation,
				boundingBox,
				materialsIndex[primitive.material]);
			primitives.push_back(std::move(prim));
		}
	}
	SceneManager::addPrimitives(std::move(primitives));
	std::cout << "Loaded " << primitives.size() << " primitives from model." << std::endl;
}

void GLTFModel::processTextures(tinygltf::Model &model)
{
	for (int i = 0; i < model.images.size(); i++)
	{
		std::string name = model.images[i].name;
		if (SceneManager::getTexture(name) == nullptr)
		{
			SceneManager::addTexture(name, std::make_shared<Tex>(model.images[i]));
		}
		texturesIndex[i] = SceneManager::getTexture(name);
	} 
}

void GLTFModel::processMaterials(tinygltf::Model &model)
{
	for (int i = 0; i < model.materials.size(); i++)
	{
		const auto& material = model.materials[i];
		Mat mat;
		if (material.pbrMetallicRoughness.baseColorTexture.index != -1)
		{
			mat.diffuse = texturesIndex[material.pbrMetallicRoughness.baseColorTexture.index];
		}
			
		if (material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
		{
			mat.specular = texturesIndex[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
		}
		if (material.normalTexture.index != -1)
		{
			mat.normal = texturesIndex[material.normalTexture.index];
		}
		mat.name = material.name;
		std::hash<std::string> hasher;
		uint32_t uid = (uint32_t)hasher(model.materials[i].name);
		if(SceneManager::getMaterial(uid) == nullptr)
		{
			SceneManager::addMaterial(std::make_shared<Mat>(mat), uid);
		}
		materialsIndex[i] = SceneManager::getMaterial(uid);
	}
}

void GLTFModel::setTransform(glm::mat4 transform)
{
	for (auto &primitive : primitives)
	{
		primitive.transform += transform;
	}
}

const std::vector<float> GLTFModel::processPosAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end())
	{
		std::cerr << "No POSITION attribute found in primitive " << mesh.name << std::endl;
	}
	const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
	const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
	const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];
	
	const float* pPosData = reinterpret_cast<const float*>(posBuffer.data.data() +
														posBufferView.byteOffset +
														posAccessor.byteOffset);

	size_t vertexCount = posAccessor.count;
	int components = (posAccessor.type == TINYGLTF_TYPE_VEC3) ? 3 : 0;
	std::vector<float> posData;
	posData.reserve(vertexCount * components);
	for (size_t i = 0; i < vertexCount; i++)
	{
		for (int j = 0; j < components; j++)
		{
			posData.push_back(pPosData[i * components + j]);
		}
	}

	return posData;

}

const std::pair<glm::vec3, glm::vec3> GLTFModel::computeBoundingBox(const tinygltf::Primitive& primitive, const tinygltf::Model& model)
{
	const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
	if (posAccessor.maxValues.size() >= 3 && posAccessor.minValues.size() >= 3) 
	{
		glm::vec3 min(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
		glm::vec3 max(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
		const std::pair<glm::vec3, glm::vec3> boundingBox = std::make_pair(min, max);
		return boundingBox;
	}

}

const std::vector<float> GLTFModel::processTexCoordAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{
	if (primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end())
	{
		std::cerr << "No TEXCOORD_0 attribute found in primitive " << mesh.name << std::endl;
	}
	const tinygltf::Accessor& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
	const tinygltf::BufferView& texBufferView = model.bufferViews[texAccessor.bufferView];
	const tinygltf::Buffer& texBuffer = model.buffers[texBufferView.buffer];

	const float* pTexCoordData = reinterpret_cast<const float*>(texBuffer.data.data() +
														texBufferView.byteOffset +
														texAccessor.byteOffset);

	size_t vertexCount = texAccessor.count;
	int components = (texAccessor.type == TINYGLTF_TYPE_VEC2) ? 2 : 0;
	std::vector<float> texCoordData;
	texCoordData.reserve(vertexCount * components);
	for (size_t i = 0; i < vertexCount; i++)
	{
		for (int j = 0; j < components; j++)
		{
			texCoordData.push_back(pTexCoordData[i * components + j]);
		}
	}
	return texCoordData;
}

const std::vector<float> GLTFModel::processNormalAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{

	if (primitive.attributes.find("NORMAL") == primitive.attributes.end())
	{
		std::cerr << "No NORMAL attribute found in primitive " << mesh.name << std::endl;
	}
	const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
	const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
	const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];

	const float* pNormalData = reinterpret_cast<const float*>(normalBuffer.data.data() +
															normalBufferView.byteOffset +
															normalAccessor.byteOffset);

	size_t vertexCount = normalAccessor.count;
	int components = (normalAccessor.type == TINYGLTF_TYPE_VEC3) ? 3 : 0;
	std::vector<float> normalData;
	normalData.reserve(vertexCount * components);
	for (size_t i = 0; i < vertexCount; i++)
	{
		for (int j = 0; j < components; j++)
		{
			normalData.push_back(pNormalData[i * components + j]);
		}
	}
	return normalData;
}

const std::vector<float> GLTFModel::processTangentNormalAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{

	if (primitive.attributes.find("TANGENT") == primitive.attributes.end())
	{
		std::cerr << "No TANGENT attribute found in primitive " << mesh.name << std::endl;
	}
	const tinygltf::Accessor& tangentNormalAccessor = model.accessors[primitive.attributes.at("TANGENT")];
	const tinygltf::BufferView& tangentNormalBufferView = model.bufferViews[tangentNormalAccessor.bufferView];
	const tinygltf::Buffer& tangentBuffer = model.buffers[tangentNormalBufferView.buffer];

	const float* pTangentNormalData = reinterpret_cast<const float*>(tangentBuffer.data.data() +
															tangentNormalBufferView.byteOffset +
															tangentNormalAccessor.byteOffset);

	size_t vertexCount = tangentNormalAccessor.count;
	int components = (tangentNormalAccessor.type == TINYGLTF_TYPE_VEC4) ? 4 : 0;
	std::vector<float> tangentNormalData;
	tangentNormalData.reserve(vertexCount * components);
	for (size_t i = 0; i < vertexCount; i++)
	{
		for (int j = 0; j < components; j++)
		{
			tangentNormalData.push_back(pTangentNormalData[i * components + j]);
		}
	}
	return tangentNormalData;
}

const std::vector<uint32_t> GLTFModel::processIndexAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{
	if (primitive.indices < 0)
	{
		std::cerr << "No indices found in primitive " << mesh.name << std::endl;
	}
	const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
	const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
	const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

	std::vector<uint32_t> indexData;

	const void *pIndexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
	size_t indexCount = indexAccessor.count;
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
	return indexData;
}
