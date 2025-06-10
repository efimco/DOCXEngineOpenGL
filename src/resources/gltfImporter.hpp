#pragma once
#include "primitive.hpp"
#include "shader.hpp"
#include "tiny_gltf.h"
#include <map>
#include <string>
#include <vector>

class GLTFModel
{
  public:
    std::string path;
    GLTFModel(std::string path);
    ~GLTFModel();

  private:
    std::vector<Primitive> primitives;
    std::map<uint32_t, std::shared_ptr<Tex>> texturesIndex;
    std::map<uint32_t, std::shared_ptr<Mat>> materialsIndex;

    tinygltf::Model readGlb(const std::string &path);
    void processGLTFModel(tinygltf::Model &model);
    void processTransforms(tinygltf::Node &node, Transform transform);
    void processTextures(tinygltf::Model &model);
    void processMaterials(tinygltf::Model &model);
    const std::pair<std::vector<float>, std::pair<glm::vec3, glm::vec3>>
    processPosAttrib(const tinygltf::Primitive &primitive,
                     const tinygltf::Mesh &mesh, const tinygltf::Model &model);
    const std::vector<float>
    processTexCoordAttrib(const tinygltf::Primitive &primitive,
                          const tinygltf::Mesh &mesh,
                          const tinygltf::Model &model);
    const std::vector<float>
    processNormalAttrib(const tinygltf::Primitive &primitive,
                        const tinygltf::Mesh &mesh,
                        const tinygltf::Model &model);
    const std::vector<float>
    processTangentNormalAttrib(const tinygltf::Primitive &primitive,
                               const tinygltf::Mesh &mesh,
                               const tinygltf::Model &model);
    const std::vector<uint32_t>
    processIndexAttrib(const tinygltf::Primitive &primitive,
                       const tinygltf::Mesh &mesh,
                       const tinygltf::Model &model);
};
