#ifndef PMX_MODEL_H_HEADER_CLASS
#define PMX_MODEL_H_HEADER_CLASS

#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "InverseKinematics.h"
#include "PMXFile.h"
#include "shader.h"
#include "Texture.h"
#include "Utils.h"



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
  int32_t addParentIndex;
  float additionalRate;
  
  glm::vec3 restPosition;
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 addTranslation;
  glm::quat addRotation;
  glm::quat ikRotation;
  glm::vec3 ikPrevAngle;

  bool hasAddTranslation;
  bool hasAddRotation;
};

struct RigidBodyModel
{
  btTransform colliderOffset;
  btCollisionShape* shape;
  btRigidBody* body;
  PMXRigidBody::OperationType operationType;
  
  int32_t relatedBoneIndex;
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
    std::vector<RigidBodyModel> rigidBody;
    std::vector<PMXRigidBody> rigidBodyPmx;
    std::vector<PMXJoint> jointsPmx;
    
    std::unordered_map<const char*, float> morphWeights;
    std::unordered_map<int, std::vector<int>> boneChildren;

    btBroadphaseInterface* physBroadphase = nullptr;
    btDefaultCollisionConfiguration* physConfig = nullptr;
    btCollisionDispatcher* physDispatcher = nullptr;
    btSequentialImpulseConstraintSolver* physSolver = nullptr;
    btDiscreteDynamicsWorld* physWorld = nullptr;

    std::vector<glm::mat4> boneMatrices;

    std::vector<glm::vec3> addTranslation;
    std::vector<glm::quat> addRotation;
    
    std::vector<glm::mat4> globalTransform;
    std::vector<glm::mat4> localTransform;

    bool hasPrint = false;
    
    PMXModel(PMXFile &pmxFile);
    void CreateRigidBody();
    void CreateJoints();
    void InitPhysics();
    void GetBoneSubtree(int index, std::vector<int> &out);
    glm::vec3 GetBoneWorldPosition(int index, bool isLog = false);
    glm::vec3 GetBoneWorldDist(int indexA, int indexB, bool isLog = false);
    glm::quat GetParentBoneWorldRot(int index, bool isLog = false);
    float GetIKChainLength(int targetIndex);
    float GetIKRelativeChainLength(int targetIndex);
    void UpdateMorph(const char* name, float &weight);
    void UpdatePhysics();
    void UpdateLocalTransform(int index);
    void UpdateAdditionalTransform(int index);
    void UpdateChildrenGlobalTransform(int index);
    void UpdateGlobalTransform(int index);
    void Update();
    void Draw(Shader& shader);
    
  private:
    
};

#endif
