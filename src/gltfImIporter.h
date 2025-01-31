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

struct Primitive 
{
	uint32_t vao;
	uint32_t vbo;
	uint32_t ebo;
	Shader shader;
	size_t indexCount;
	glm::mat4 transform;

	Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, Shader shader, size_t indexCount, glm::mat4 transform)
		: vao(vao), vbo(vbo), ebo(ebo), shader(shader), indexCount(indexCount), transform(transform) {}

	Primitive(const Primitive&) = delete;
	Primitive& operator=(const Primitive&) = delete;

	Primitive(Primitive&& other) noexcept
		: vao(other.vao), vbo(other.vbo), ebo(other.ebo), shader(std::move(other.shader)),
		indexCount(other.indexCount), transform(other.transform) {
		other.vao = 0;
		other.vbo = 0;
		other.ebo = 0;
	}
	
	~Primitive()
	{
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
	}
	
	void draw(Camera& camera, int32_t width, int32_t height)
	{
		const auto view = camera.getViewMatrix();
		const auto projection = glm::perspective(glm::radians(camera.zoom), float(width)/float(height),0.1f, 100.0f);	

		shader.use();
		shader.setMat4("projection",projection);
		shader.setMat4("view",view);
		shader.setMat4("model",transform);
		
		glBindVertexArray(vao);
		int eboSize = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
		int indexSize = eboSize / sizeof(int);
		glDrawElements(GL_TRIANGLES,indexSize,GL_UNSIGNED_INT,(void*)0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}
};

class GLTFModel
{
	public:
		Shader shader;
		std::vector<Primitive> primitives;
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
			for ( auto &mesh : model.meshes) 
			{
				for ( auto &primitive : mesh.primitives) 
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
					size_t vertexCount = posAccessor.count;
					int componentCount = posAccessor.type == TINYGLTF_TYPE_VEC3? 3:0;
					for (int i = 0; i < vertexCount; i++)
					{
						for (int j = 0; j < componentCount; j++)
						{
							posData.push_back(pPosData[i * componentCount + j]);
						}
					}

					size_t indexCount = 0;
					std::vector<uint32_t> indexData;
					if (primitive.indices >= 0) 
					{
						const auto &indexAccessor = model.accessors[primitive.indices];
						const auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
						const auto &indexBuffer = model.buffers[indexBufferView.buffer];

						const void *pIndexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;
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

					uint32_t vao = 0, vbo = 0, ebo = 0;
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
					primitives.emplace_back(vao, vbo, ebo, shader, indexCount, glm::mat4(1));
				}
			}
		}

		void setup()
		{
			auto model = readGlb(path);
			processGLTFModel(model);
		}

		void setTransform(glm::mat4 transform)
		{
			for (auto& primitive: primitives)
			{
				primitive.transform = transform;
			}
		}
};
