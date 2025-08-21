#ifndef PMX_FILE_H_HEADER_CLASS
#define PMX_FILE_H_HEADER_CLASS


#include <cstdint>
#include <codecvt>
#include <filesystem>
#include <iostream>
#include <locale>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

#include<glm/glm.hpp>



struct PMXHeader
{
  char signature[4];
  float version;
};

struct PMXInfo
{
  uint8_t dataCount;
  uint8_t encoding;
  uint8_t additionalUV;
};

struct PMXIndexSize
{
  uint8_t vertex;
  uint8_t texture;
  uint8_t material;
  uint8_t bone;
  uint8_t morph;
  uint8_t rigid;
};

struct Vec2
{
  float x, y;
};

struct Vec3
{
  float x, y, z;
};

struct Vec4
{
  float x, y, z, w;
};

struct PMXVertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
  std::vector<glm::vec4> additionalUVs;

  uint8_t weightType; // 0:BDEF1 1:BDEF2 2:BDEF4 3:SDEF 4:QDEF
  std::vector<int32_t> boneIndices;
  std::vector<float> weights;
  
  glm::vec3 C, R0, R1;

  float edgeScale;
};


enum class DrawingModes : uint8_t
{
  DISABLE_CULLING   = 0x0001,
  CASTS_SHADOW      = 0x0002,
  CASTS_SHADOW_MAP  = 0x0004,
  RECEIVES_SHADOW   = 0x0008,
  ENABLE_EDGE       = 0x0010
};


enum class EnvironmentModes: uint8_t
{
  NONE,
  MULTIPLY,
  ADDITIVE,
  SUBTRACT,
};


enum class ToonIndex: uint8_t
{
  TEXTURE,
  INBUILT,
};


struct PMXMaterial
{
  std::string nameLocal;
  std::string nameGlobal;
  
  glm::vec4 diffuseColor;
  glm::vec3 specularColor;
  float specularity;
  glm::vec3 ambientColor;
  
  uint8_t drawingMode;
  
  glm::vec4 edgeColor;
  float edgeSize;
  
  int32_t textureIndex;
  
  int32_t environmentIndex;
  uint8_t environmentMode;
  
  uint8_t toonFlag;
  int32_t toonIndex;
  
  std::string memo;
  
  int32_t faceCount;
};


enum BoneFlag : uint16_t {
  CONNECTION                = 0x0001,
  ROTATABLE                 = 0x0002,
  MOVABLE                   = 0x0004,
  DISPLAY_FLAG              = 0x0008,
  CAN_OPERATE               = 0x0010,
  INVERSE_KINEMATICS        = 0x0020,
  RESERVED_BIT              = 0x0040, // Unused bit
  ADD_LOCAL_DEFORM          = 0x0080,
  ADD_ROTATION              = 0x0100,
  ADD_MOVEMENT              = 0x0200,
  FIXED_AXIS                = 0x0400,
  LOCAL_AXIS                = 0x0800,
  PHYSICAL_TRANSFORM        = 0x1000,
  EXTERNAL_PARENT_TRANSFORM = 0x2000,
};


struct PMXIKBoneLink
{
  int32_t ikBoneIndex;
  uint8_t enableAngleLimit;
  glm::vec3 lowerLimit;
  glm::vec3 upperLimit;
};


struct PMXBone
{
  std::string nameLocal;
  std::string nameGlobal;
  
  glm::vec3 position;
  int32_t parentBoneIndex;
  
  int32_t transformLevel;
  
  int16_t boneFlag;
  
  // Set connection
  int32_t connectionIndex;
  
  // Unset connection
  glm::vec3 positionOffset;
  
  // Set add rotation | add movement
  int32_t additionalParentIndex;
  float additionalRate;
  
  // Set fixed axis
  glm::vec3 axisVector;
  
  // Set local axis
  glm::vec3 xAxisVector;
  glm::vec3 zAxisVector;
  
  // Set external parent transform
  int32_t keyValue;
  
  // Set inverse kinematics
  int32_t ikBoneIndex;
  int32_t ikIteration;
  float ikLimitAngle;
  int32_t ikLinkCount;
  std::vector<PMXIKBoneLink> ikLinks;
};

enum class MorphType : uint8_t
{
  GROUP,
  VERTEX,
  BONE,
  UV,
  ADDITIONAL_UV1,
  ADDITIONAL_UV2,
  ADDITIONAL_UV3,
  ADDITIONAL_UV4,
  MATERIAL,
  FLIP,
  IMPULSE,
};

struct PMXMorph
{
  std::string nameLocal;
  std::string nameGlobal;
  
  uint8_t handlePanel; //1: Eyebrows (Bottom left) 2: Eyes (Top left) 3: Mouth (Top right) 4: Other (Bottom right)  | 0: Hidden (None)
  MorphType morphType;
  int32_t morphOffsetCount;
  
  struct GroupMorph
  {
    int32_t groupIndex;
    float groupRate;
  };
  
  struct FlipMorph
  {
    int32_t flipIndex;
    float flipRate;
  };
  
  struct VertexMorph
  {
    int32_t vertexIndex;
    glm::vec3 positionOffset;
  };
  
  struct BoneMorph
  {
    int32_t boneIndex;
    glm::vec3 moveValue;
    glm::vec4 rotationValue;
  };
  
  struct UVMorph
  {
    int32_t vertexIndex;
    glm::vec4 uvOffset;
  };
  
  struct MaterialMorph
  {
    int32_t materialIndex;
    uint8_t offsetMethod;
    
    glm::vec4 diffuseColor;
    glm::vec3 specularColor;
    float specularity;
    glm::vec3 ambientColor;
    
    glm::vec4 edgeColor;
    float edgeSize;
    
    glm::vec4 textureTint;
    
    glm::vec4 environmentTint;
    
    glm::vec4 toonTint;
  };
  
  struct ImpulseMorph
  {
    int32_t rigidBodyIndex;
    uint8_t localFlag;
    glm::vec3 moreVelocity;
    glm::vec3 rotationTorque;
  };
  
  std::vector<GroupMorph> groupMorph;
  std::vector<FlipMorph> flipMorph;
  std::vector<VertexMorph> vertexMorph;
  std::vector<BoneMorph> boneMorph;
  std::vector<UVMorph> uvMorph;
  std::vector<MaterialMorph> materialMorph;
  std::vector<ImpulseMorph> impulseMorph;
};


struct PMXFrame
{
  std::string nameLocal;
  std::string nameGlobal;
  
  enum class FrameType: uint8_t
  {
    DEFAULT_FRAME,
    SPECIAL_FRAME,
  };
  
  FrameType specialFrame; // 0: regular frame 1: special frame
  int32_t elementCount;
  
  enum class TargetType: uint8_t
  {
    BONE_INDEX,
    MORPH_INDEX,
  };
  
  struct Target
  {
    TargetType targetType;
    int32_t targetIndex;
  };
  
  std::vector<Target> targets;
};


struct PMXRigidBody
{
  std::string nameLocal;
  std::string nameGlobal;
  
  int32_t relatedBoneIndex;
  uint8_t groupIndex;
  int16_t ignoreCollisionGroup;
  
  enum class ShapeType: uint8_t
  {
    SPHERE,
    BOX,
    CAPSULE,
  };
  
  ShapeType shapeType;
  glm::vec3 shapeSize;
  
  glm::vec3 colliderPosition;
  glm::vec3 colliderRotation;
  
  float weight;
  
  float positionAttenuation;
  float rotationAttenuation;
  
  float recoil;
  float friction;
  
  enum class OperationType: uint8_t
  {
    STATIC, // "Jelly" style bone follow
    DYNAMIC, // Real physics
    DYNAMIC_POSITION_ADJUST, // Real physics affecting bone
  };
  
  OperationType operationType;
};

struct PMXJoint
{
  std::string nameLocal;
  std::string nameGlobal;
  
  uint8_t operationType;
  
  int32_t rigidBodyIndex1;
  int32_t rigidBodyIndex2;
  
  glm::vec3 position;
  glm::vec3 rotation;
  
  glm::vec3 positionConstraintLower;
  glm::vec3 positionConstraintUpper;
  
  glm::vec3 rotationConstraintLower;
  glm::vec3 rotationConstraintUpper;
  
  glm::vec3 springPosition;
  glm::vec3 springRotation;
};


template <typename T>
bool readBinary(std::istream &stream, T &value);

// PMX specification reference:
// https://gist.github.com/felixjones/f8a06bd48f9da9a4539f/b3944390bd935f48ddf72dd2fc058ffe87c10708
class PMXFile
{
  public:
    std::ifstream file;
  
    PMXHeader header;
    PMXInfo info;
    PMXIndexSize indexSize;
    
    std::string localName;
    std::string globalName;
    std::string localComment;
    std::string globalComment;
    
    std::vector<PMXVertex> vertices;
    std::vector<int32_t> indices;
    std::vector<std::string> textures;
    std::vector<PMXMaterial> materials;
    std::vector<PMXBone> bones;
    std::vector<PMXMorph> morphs;
    std::vector<PMXFrame> frames;
    std::vector<PMXRigidBody> rigidBodies;
    std::vector<PMXJoint> joints;
    
    PMXFile(const char* filename);
    
  
  private:
    std::string readPmxString(std::ifstream &file, uint8_t encoding);
    int32_t readPmxIndex(std::ifstream &file, uint8_t size);
    PMXVertex readPmxVertex(std::ifstream &file, uint8_t additionalUVCount, uint8_t boneIndexSize);
    PMXMaterial readPMXMaterial(std::ifstream &file, uint8_t textureIndexSize, uint8_t encoding);
    PMXBone readPMXBone(std::ifstream &file, uint8_t boneIndexSize, uint8_t encoding);
    PMXMorph readPMXMorph(std::ifstream &file, uint8_t morphIndexSize, uint8_t vertexIndexSize, uint8_t boneIndexSize, uint8_t materialIndexSize, uint8_t rigidIndexSize, uint8_t encoding);
    PMXFrame readPMXFrame(std::ifstream &file, uint8_t boneIndexSize, uint8_t morphIndexSize, uint8_t encoding);
    PMXRigidBody readPMXRigidBody(std::ifstream &file, uint8_t boneIndexSize, uint8_t encoding);
    PMXJoint readPMXJoint(std::ifstream &file, uint8_t rigidBodyIndexSize, uint8_t encoding);
};




#endif