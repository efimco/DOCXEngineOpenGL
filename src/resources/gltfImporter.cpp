#include "gltfImporter.hpp"
#include "glm/gtx/quaternion.hpp"
#include "scene/model.hpp"
#include "sceneManager.hpp"
#include <glad/gl.h>
#include <iostream>
#include <iterator>
#include <string>

GLTFModel::GLTFModel(std::string path) : path(path)
{
	auto model = readGlb(path);
	processTextures(model);
	processMaterials(model);
	processGLTFModel(model);
}

GLTFModel::~GLTFModel() { std::cout << "GLTF Importer destructor called!\n" << std::endl; }

tinygltf::Model GLTFModel::readGlb(const std::string& path)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;
	printf("Loading...%s\n ", path.c_str());
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

void GLTFModel::processGLTFModel(tinygltf::Model& model)
{
	for (const auto& mesh : model.meshes)
	{
		for (const auto& primitive : mesh.primitives)
		{
			std::pair<std::vector<float>, std::pair<glm::vec3, glm::vec3>> posAndBB =
				processPosAttrib(primitive, mesh, model);
			std::vector<float> posBuffer = posAndBB.first;
			std::pair<glm::vec3, glm::vec3> boundingBox = posAndBB.second;
			std::vector<float> texBuffer = processTexCoordAttrib(primitive, mesh, model);
			std::vector<float> normalBuffer = processNormalAttrib(primitive, mesh, model);
			std::vector<uint32_t> indexBuffer = processIndexAttrib(primitive, mesh, model);
			if (posBuffer.empty() || normalBuffer.empty())
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
			size_t bufferSize = posSize + texSize + normalSize;

			size_t indexSize = indexBuffer.size() * sizeof(uint32_t);

			glCreateBuffers(1, &ebo);
			glNamedBufferData(ebo, indexBuffer.size() * sizeof(uint32_t), indexBuffer.data(), GL_STATIC_DRAW);
			glVertexArrayElementBuffer(vao, ebo);

			glNamedBufferData(vbo, bufferSize, NULL, GL_STATIC_DRAW); //alocate enough space for all the date
			glNamedBufferSubData(vbo, 0, posSize, posBuffer.data()); //fill the data starting from the point where previous data ended
			glNamedBufferSubData(vbo, posSize, texSize, texBuffer.data());
			glNamedBufferSubData(vbo, posSize + texSize, normalSize, normalBuffer.data());

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

			Transform transform;
			size_t meshIndex = &mesh - &model.meshes[0];
			if (model.nodes[meshIndex].translation.size() != 0)
			{
				transform.position = glm::vec3(model.nodes[meshIndex].translation[0], model.nodes[meshIndex].translation[1],
					model.nodes[meshIndex].translation[2]);
			}
			else
			{
				transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			if (model.nodes[meshIndex].rotation.size() != 0)
			{
				glm::quat quatRot(static_cast<float>(model.nodes[meshIndex].rotation[3]),
					static_cast<float>(model.nodes[meshIndex].rotation[0]),
					static_cast<float>(model.nodes[meshIndex].rotation[1]),
					static_cast<float>(model.nodes[meshIndex].rotation[2]));
				transform.rotation = glm::eulerAngles(quatRot);
			}
			else
			{
				transform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
			}

			if (model.nodes[meshIndex].scale.size() != 0)
			{
				transform.scale = glm::vec3(model.nodes[meshIndex].scale[0], model.nodes[meshIndex].scale[1], model.nodes[meshIndex].scale[2]);
			}
			else
			{
				transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
			}
			transform.matrix = glm::translate(glm::mat4(1.0f), transform.position) * glm::mat4_cast(transform.rotation) *
				glm::scale(glm::mat4(1.0f), transform.scale);

			assert(indexBuffer.size() == model.accessors[primitive.indices].count);
			size_t indexCount = indexBuffer.size();
			std::string name = mesh.name;
			auto it = std::find_if(primitives.begin(), primitives.end(), [&](const auto& prim) { return prim->name == name; });

			if (it != primitives.end())
			{
				std::cerr << "Warning: Duplicate primitive names were detected: " << name << std::endl;
				static int duplicateCounter = 0;
				name.append(std::to_string(duplicateCounter));
				duplicateCounter++;
			}

			std::shared_ptr<Mat> mat = std::make_shared<Mat>();

			if (primitive.material < 0)
			{
				mat->name = name;
				std::hash<std::string> hasher;
				uint32_t uid = (uint32_t)hasher(mat->name);
				if (SceneManager::getMaterial(uid) == nullptr)
				{
					SceneManager::addMaterial(mat, uid);
				}
			}
			else
			{
				mat = materialsIndex[primitive.material];
			}
			std::unique_ptr<Primitive> newPrimtive =
				std::make_unique<Primitive>(vao, vbo, ebo, indexCount, transform, boundingBox, mat, name);
			primitives.push_back(std::move(newPrimtive));
		}
	}
	for (const auto& primtive : primitives)
	{
		SceneManager::addPrimitive(primtive.get());
	}

	std::cout << "Loaded " << primitives.size() << " primitives from model." << std::endl;
}

std::unique_ptr<SceneGraph::Model> GLTFModel::getModel()
{
	Transform modelTransform;
	modelTransform.matrix = glm::mat4(1.0f);
	modelTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
	modelTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	modelTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
	std::string modelName = this->path.substr(path.find_last_of("/\\") + 1);
	std::unique_ptr<SceneGraph::Model> finalModel = std::make_unique<SceneGraph::Model>(modelTransform, modelName);

	for (auto& primitive : primitives)
	{
		finalModel->addChild(std::unique_ptr<SceneNode>(primitive.release()));
	}
	return finalModel;
}

void GLTFModel::processTextures(tinygltf::Model& model)
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

void GLTFModel::processMaterials(tinygltf::Model& model)
{
	for (int i = 0; i < model.materials.size(); i++)
	{
		const auto& material = model.materials[i];
		std::shared_ptr<Mat> mat = std::make_shared<Mat>();
		if (material.pbrMetallicRoughness.baseColorTexture.index != -1)
		{
			mat->tDiffuse = texturesIndex[material.pbrMetallicRoughness.baseColorTexture.index];
		}

		if (material.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
		{
			mat->tSpecular = texturesIndex[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
		}
		if (material.normalTexture.index != -1)
		{
			mat->tNormal = texturesIndex[material.normalTexture.index];
		}
		mat->name = material.name;
		if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
			mat->albedo = glm::vec4(
				static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]),
				static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]),
				static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]),
				static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3])
			);
		}
		std::hash<std::string> hasher;
		uint32_t uid = (uint32_t)hasher(model.materials[i].name);
		if (SceneManager::getMaterial(uid) == nullptr)
		{
			SceneManager::addMaterial(mat, uid);
		}
		materialsIndex[i] = SceneManager::getMaterial(uid);
	}
	if (model.materials.empty())
	{
		std::cerr << "No materials found in the model." << std::endl;
	}
	else
	{
		std::cout << "Processed " << model.materials.size() << " materials." << std::endl;
	}
}

const std::pair<std::vector<float>, std::pair<glm::vec3, glm::vec3>>
GLTFModel::processPosAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh, const tinygltf::Model& model)
{
	if (primitive.attributes.find("POSITION") == primitive.attributes.end())
	{
		std::cerr << "No POSITION attribute found in primitive " << mesh.name << std::endl;
	}
	const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
	const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
	const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];

	const float* pPosData =
		reinterpret_cast<const float*>(posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset);

	size_t vertexCount = posAccessor.count;
	int components = (posAccessor.type == TINYGLTF_TYPE_VEC3) ? 3 : 0;
	std::vector<float> posData;
	posData.reserve(vertexCount * components);
	glm::vec3 minPos(FLT_MAX);
	glm::vec3 maxPos(-FLT_MAX);
	for (size_t i = 0; i < vertexCount; i++)
	{
		for (int j = 0; j < components; j++)
		{
			posData.push_back(pPosData[i * components + j]);
			pPosData[i * components + j];
			minPos[j] = std::min(minPos[j], pPosData[i * components + j]);
			maxPos[j] = std::max(maxPos[j], pPosData[i * components + j]);
		}
	}
	std::pair<glm::vec3, glm::vec3> boundingBox(minPos, maxPos);
	return std::make_pair(posData, boundingBox);
}

const std::vector<float> GLTFModel::processTexCoordAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh,
	const tinygltf::Model& model)
{
	if (primitive.attributes.find("TEXCOORD_0") == primitive.attributes.end())
	{
		std::cerr << "No TEXCOORD_0 attribute found in primitive " << mesh.name << std::endl;
		return std::vector<float>(0);
	}
	const tinygltf::Accessor& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
	const tinygltf::BufferView& texBufferView = model.bufferViews[texAccessor.bufferView];
	const tinygltf::Buffer& texBuffer = model.buffers[texBufferView.buffer];

	const float* pTexCoordData =
		reinterpret_cast<const float*>(texBuffer.data.data() + texBufferView.byteOffset + texAccessor.byteOffset);

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

const std::vector<float> GLTFModel::processNormalAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh,
	const tinygltf::Model& model)
{

	if (primitive.attributes.find("NORMAL") == primitive.attributes.end())
	{
		std::cerr << "No NORMAL attribute found in primitive " << mesh.name << std::endl;
		return std::vector<float>(0);
	}
	const tinygltf::Accessor& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
	const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
	const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];

	const float* pNormalData =
		reinterpret_cast<const float*>(normalBuffer.data.data() + normalBufferView.byteOffset + normalAccessor.byteOffset);

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

const std::vector<uint32_t> GLTFModel::processIndexAttrib(const tinygltf::Primitive& primitive, const tinygltf::Mesh& mesh,
	const tinygltf::Model& model)
{
	if (primitive.indices < 0)
	{
		std::cerr << "No indices found in primitive " << mesh.name << std::endl;
		return std::vector<uint32_t>(0);
	}
	const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
	const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
	const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

	std::vector<uint32_t> indexData;

	const void* pIndexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
	size_t indexCount = indexAccessor.count;
	if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		const uint16_t* indices = reinterpret_cast<const uint16_t*>(pIndexData);
		indexData.assign(indices, indices + indexCount);
	}
	else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
	{
		const uint32_t* indices = reinterpret_cast<const uint32_t*>(pIndexData);
		indexData.assign(indices, indices + indexCount);
	}
	else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
	{
		const uint8_t* indices = reinterpret_cast<const uint8_t*>(pIndexData);
		indexData.assign(indices, indices + indexCount);
	}
	return indexData;
}
