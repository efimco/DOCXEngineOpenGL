#pragma once
#include "shader.h"
#include <vector>
#include "glm/glm.hpp"
#include "camera.h"
#include "material.h"
class Primitive 
{
	public:
		uint32_t vao;
		uint32_t vbo;
		uint32_t ebo;
		Shader shader;
		Shader outlineShader;
		size_t indexCount;
		glm::mat4 transform;
		bool selected;
		Mat material;

		Primitive(uint32_t vao, uint32_t vbo, uint32_t ebo, Shader shader, size_t indexCount, glm::mat4 transform, Mat material)
			: vao(vao), vbo(vbo), ebo(ebo), shader(shader), indexCount(indexCount), transform(transform),
			outlineShader(std::filesystem::absolute("..\\..\\src\\shaders\\outlineVert.glsl").string(),
							std::filesystem::absolute("..\\..\\src\\shaders\\outlineFrag.glsl").string()),
			selected(false), material(material){};

		Primitive(const Primitive&) = delete;
		Primitive& operator=(const Primitive&) = delete;


		Primitive(Primitive&& other) noexcept
			:vao(other.vao),
			vbo(other.vbo),
			ebo(other.ebo),
			shader(std::move(other.shader)),
			indexCount(other.indexCount),
			transform(other.transform),
			material(std::move(other.material)),
			outlineShader(std::filesystem::absolute("..\\..\\src\\shaders\\outlineVert.glsl").string(),
							std::filesystem::absolute("..\\..\\src\\shaders\\outlineFrag.glsl").string()),
							selected(false) 
			{
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
			const auto projection = glm::perspective(glm::radians(camera.zoom), float(width == 0 ? 1 : width)/float(height == 0 ? 1 : height),0.1f, 100.0f);	
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
			shader.use();
			shader.setVec3("objectIDColor", setPickColor(vao));
			if (material.diffuse -> path != "")
			{
				shader.setInt(material.diffuse -> type, 1);
				glBindTextureUnit(1, material.diffuse -> id);
			}else glBindTextureUnit(1, 0);
			if (material.specular -> path != "")
			{
				shader.setInt(material.specular -> type, 2);
				shader.setFloat("shininess", 32);
				glBindTextureUnit(2, material.specular -> id);
			}else glBindTextureUnit(2, 0);
			shader.setMat4("projection",projection);
			shader.setMat4("view",view);
			shader.setMat4("model",transform);
			
			glBindVertexArray(vao);
			int eboSize = 0;
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
			int indexSize = eboSize / sizeof(int);
			glDrawElements(GL_TRIANGLES,indexSize,GL_UNSIGNED_INT,(void*)0);
			glActiveTexture(GL_TEXTURE0);
			if (selected == true)
			{
				glEnable(GL_STENCIL_TEST);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0x00); 
				float camDistancece = (float)glm::length(camera.position - glm::vec3(transform[3]));
				float outlineScale = 1.025f;
				scaledTransform = glm::scale(transform, glm::vec3(outlineScale));
				outlineShader.use();
				outlineShader.setMat4("projection",projection);
				outlineShader.setMat4("view",view);
				outlineShader.setMat4("model",scaledTransform);
				glDrawElements(GL_TRIANGLES,indexSize,GL_UNSIGNED_INT,(void*)0);
				glStencilMask(0xFF);
				glStencilFunc(GL_ALWAYS, 1, 0xFF); 
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_STENCIL_TEST);
			}
			
			glBindVertexArray(0);
		}
	private:
		glm::vec3 setPickColor(unsigned int id)
		{
			float golden_ratio_conjugate = 0.618033988749895f;
			float h = glm::fract(id * golden_ratio_conjugate);
			float s = 0.7f; // moderately saturated
			float v = 1.0f; // bright

			glm::vec3 idColor = hsv2rgb(h,s,v);
			return idColor;
		}

		glm::vec3 hsv2rgb(float h, float s, float v) {
			float c = v * s;
			float h_prime = h * 6.0f;
			float x = c * (1.0f - fabs(fmod(h_prime, 2.0f) - 1.0f));
			glm::vec3 rgb;
			
			if (h_prime < 1.0f)
				rgb = glm::vec3(c, x, 0.0f);
			else if (h_prime < 2.0f)
				rgb = glm::vec3(x, c, 0.0f);
			else if (h_prime < 3.0f)
				rgb = glm::vec3(0.0f, c, x);
			else if (h_prime < 4.0f)
				rgb = glm::vec3(0.0f, x, c);
			else if (h_prime < 5.0f)
				rgb = glm::vec3(x, 0.0f, c);
			else
				rgb = glm::vec3(c, 0.0f, x);
			
			float m = v - c;
			glm::vec3 res = rgb + glm::vec3(m);
			printf("%f %f %f\n", res.x, res.y, res.z);
			return res;
		}
		// Converts an RGB color (each channel in [0,1]) back to HSV

		glm::mat4 scaledTransform;
};