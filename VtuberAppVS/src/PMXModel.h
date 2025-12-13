#ifndef PMX_MODEL_H_HEADER_CLASS
#define PMX_MODEL_H_HEADER_CLASS

#include<algorithm>
#include<set>
#include<unordered_map>
#include<unordered_set>

#include<glad/glad.h>
#include<glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "PMXFile.h"
#include "shader.h"
#include "Texture.h"



struct VertexModel
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texUV;
  glm::ivec4 boneIndices;
  glm::vec4 boneWeights;
};

struct BoneModel
{
  std::string nameLocal;
  std::string nameGlobal;
  
  int32_t parentBoneIndex;
  
  glm::vec3 restPosition;
  
  glm::vec3 position;
  glm::vec3 rotation;
};

class PMXModel
{
  public:
    GLuint VAO, VBO, EBO;
    std::vector<VertexModel> baseVertices;
    std::vector<VertexModel> vertices;
    std::vector<VertexModel> skinnedVertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    std::vector<PMXMaterial> materials;
    std::vector<BoneModel> bones;
    std::vector<PMXBone> bonesPmx;
    std::vector<PMXMorph> morphs;
    
    std::unordered_map<int, std::vector<int>> boneChildren;
    
    std::vector<glm::mat4> boneMatrices;
    
    std::vector<glm::mat4> globalTransform;
    std::vector<glm::mat4> localTransform;
    
    PMXModel(PMXFile &pmxFile);
    void UpdateMorph(float &weight);
    void Update();
    void Draw(Shader& shader);
    
  private:
    
};

#endif