#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef APIENTRY
#undef APIENTRY
#endif

#include "glad/glad.h"
#include "tiny_gltf.h"
#include "shader.h"
#include "camera.h"
#include <map>

class GLTFModel
{
	public:

		glm::mat4 model;
		Shader shader;
		std::vector<uint32_t> vaos;
		std::map<uint32_t, Shader> primitiveMap;
		std::string path;
		GLTFModel(std::string Path, const Shader& shader):path(Path), shader(shader) 
		{
			setup();
		}
		
		tinygltf::Model readGlb(std::string path)
		{		
			tinygltf::Model model;
			tinygltf::TinyGLTF loader;
			std::string err;
			std::string warn;
			bool ret = loader.LoadASCIIFromFile(&model, &err, &warn,path);
			if (!warn.empty()) printf("Warn: %s\n", warn.c_str());
			if (!err.empty()) printf("Err: %s\n", err.c_str());
			if (!ret) printf("Failed to parse glTF\n");
			return model;
		}


		void processGLTFModel(tinygltf::Model &model)
		{
			for (const auto &mesh : model.meshes) 
			{
				for (const auto &primitive : mesh.primitives) 
				{	
					if (primitive.attributes.find("POSITION") == primitive.attributes.end()) 
					{
						std::cerr << "No POSITION attribute found in mesh " << mesh.name << std::endl;
						continue;
					}
		
					const auto &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
					const auto &posBufferView = model.bufferViews[posAccessor.bufferView];
					const auto &posBuffer = model.buffers[posBufferView.buffer];
					const float* pPosData = reinterpret_cast<const float*> (posBuffer.data.data() + posBufferView.byteOffset + posAccessor.byteOffset);

					std::vector<float> posData;
					int vertexCount = posAccessor.count;
					int componentCount = posAccessor.type == TINYGLTF_TYPE_VEC3? 3:0;
					for (int i = 0; i < vertexCount; i++)
					{
						for (int j = 0; j < componentCount; j++)
						{
							posData.push_back(pPosData[i * componentCount + j]);
						}
						
					}
					
					std::vector<uint32_t> indexData;
					if (primitive.indices >= 0) 
					{
						const auto &indexAccessor = model.accessors[primitive.indices];
						const auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
						const auto &indexBuffer = model.buffers[indexBufferView.buffer];

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


					}

					uint32_t vao, vbo, ebo;
					glCreateBuffers(1, &vbo);
					glNamedBufferData(vbo, posData.size() * sizeof(float), posData.data(), GL_STATIC_DRAW);

					if (!indexData.empty()) 
					{
					glCreateBuffers(1, &ebo);
					glNamedBufferData(ebo, indexData.size() * sizeof(uint32_t), indexData.data(), GL_STATIC_DRAW);
					}

					glCreateVertexArrays(1, &vao);
					glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(float) * 3);
					glEnableVertexArrayAttrib(vao, 0);
					glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
					glVertexArrayAttribBinding(vao, 0, 0);
					if (ebo) 
					{
					glVertexArrayElementBuffer(vao, ebo);
					}

					glBindVertexArray(0);
					primitiveMap.emplace(vao,Shader(shader.vPath,shader.fPath));

				}
			}
		}

		void setup()
		{
			model = glm::mat4(1.0f);
			auto model = readGlb(path);
			processGLTFModel(model);
		}

		void draw(Camera& camera, int32_t width, int32_t height)
		{
			for (const auto& [vao, primitiveShader] : primitiveMap)
			{
				const auto view = camera.getViewMatrix();
				const auto projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	

				primitiveShader.use();
				primitiveShader.setMat4("projection",projection);
				primitiveShader.setMat4("view",view);
				primitiveShader.setMat4("model",model);
				
				glBindVertexArray(vao);
				int eboSize = 0;
				glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
				int indexSize = eboSize / sizeof(int);
				glDrawElements(GL_TRIANGLES,indexSize,GL_UNSIGNED_INT,(void*)0);
				glBindVertexArray(0);
				glActiveTexture(GL_TEXTURE0);
			}
		}

};








