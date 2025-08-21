#include "PMXFile.h"



template <typename T>
bool readBinary(std::istream &stream, T &value)
{
  stream.read(reinterpret_cast<char*>(&value), sizeof(T));
  return stream.good();
}


std::string PMXFile::readPmxString(std::ifstream &file, uint8_t encoding)
{
  int32_t length = 0;
  file.read(reinterpret_cast<char *>(&length), 4);

  if (length <= 0)
    return "";

  std::vector<char> buffer(length);
  file.read(buffer.data(), length);
  
  if (encoding == 0)
  {
    // buffer contains UTF-16LE data
    auto utf16 = reinterpret_cast<const wchar_t*>(buffer.data());
    int utf16Length = length / sizeof(wchar_t);

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16Length, nullptr, 0, nullptr, nullptr);

    // Convert UTF-16LE -> UTF-8
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16, utf16Length, &result[0], sizeNeeded, nullptr, nullptr);

    return result;
  }
  else if (encoding == 1)
  {
    // UTF-8 → return directly
    return std::string(buffer.begin(), buffer.end());
  }

  return "";
}


int32_t PMXFile::readPmxIndex(std::ifstream &file, uint8_t size)
{
  switch (size)
  {
  case 1:
  {
    int8_t v;
    readBinary(file, v);
    return v;
  }
  case 2:
  {
    int16_t v;
    readBinary(file, v);
    return v;
  }
  case 4:
  {
    int32_t v;
    readBinary(file, v);
    return v;
  }
  default:
    throw std::runtime_error("Invalid PMX index size");
  }
}


PMXVertex PMXFile::readPmxVertex(std::ifstream &file, uint8_t additionalUVCount, uint8_t boneIndexSize)
{
  PMXVertex v;

  readBinary(file, v.position);
  readBinary(file, v.normal);
  readBinary(file, v.uv);

  for (int i = 0; i < additionalUVCount; ++i)
  {
    glm::vec4 uv;
    readBinary(file, uv);
    v.additionalUVs.push_back(uv);
  }

  readBinary(file, v.weightType);

  switch (v.weightType)
  {
  case 0:
  { // BDEF1
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    v.weights.push_back(1.0f); // Full weight to one bone
    break;
  }
  case 1:
  { // BDEF2
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    float w;
    readBinary(file, w);
    v.weights = {w, 1.0f - w};
    break;
  }
  case 2:
  case 4:
  {
    // BDEF4 and QDEF
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));

    float w1, w2, w3, w4;
    
    readBinary(file, w1);
    readBinary(file, w2);
    readBinary(file, w3);
    readBinary(file, w4);

    v.weights.push_back(w1);
    v.weights.push_back(w2);
    v.weights.push_back(w3);
    v.weights.push_back(w4);

    break;
  }
  case 3:
  {
    // SDEF
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));
    v.boneIndices.push_back(readPmxIndex(file, boneIndexSize));

    float w;
    readBinary(file, w);
    v.weights.push_back(w);

    readBinary(file, v.C);
    readBinary(file, v.R0);
    readBinary(file, v.R1);

    break;
  }
  default:
    throw std::runtime_error("Unsupported weight type in PMX vertex");
  }
  
  // Edge scale
  readBinary(file, v.edgeScale);

  return v;
}


PMXMaterial PMXFile::readPMXMaterial(std::ifstream &file, uint8_t textureIndexSize, uint8_t encoding)
{
  PMXMaterial m;
  
  m.nameLocal = readPmxString(file, encoding);
  m.nameGlobal = readPmxString(file, encoding);
  
  readBinary(file, m.diffuseColor);
  readBinary(file, m.specularColor);
  readBinary(file, m.specularity);
  readBinary(file, m.ambientColor);
  
  readBinary(file, m.drawingMode);
  
  readBinary(file, m.edgeColor);
  readBinary(file, m.edgeSize);
  
  m.textureIndex = readPmxIndex(file, textureIndexSize);
  
  m.environmentIndex = readPmxIndex(file, textureIndexSize);
  readBinary(file, m.environmentMode);
  
  readBinary(file, m.toonFlag);
  m.toonIndex = readPmxIndex(file, textureIndexSize);
  
  m.memo = readPmxString(file, encoding);
  
  readBinary(file, m.faceCount);
  // std::cout << "Texture count: " << m.faceCount << std::endl;
  
  return m;
}


PMXBone PMXFile::readPMXBone(std::ifstream &file, uint8_t boneIndexSize, uint8_t encoding)
{
  PMXBone b;
  
  b.nameLocal = readPmxString(file, encoding);
  b.nameGlobal = readPmxString(file, encoding);
  
  readBinary(file, b.position);
  b.parentBoneIndex = readPmxIndex(file, boneIndexSize);
  
  readBinary(file, b.transformLevel);
  
  readBinary(file, b.boneFlag);
  
  // Connection
  if (b.boneFlag & BoneFlag::CONNECTION)
  {
    b.connectionIndex = readPmxIndex(file, boneIndexSize);
  }
  else
  {
    readBinary(file, b.positionOffset);
  }
  
  // Add Rotation | Add Movement
  if ( (b.boneFlag & BoneFlag::ADD_ROTATION) || (b.boneFlag & BoneFlag::ADD_MOVEMENT) )
  {
    b.additionalParentIndex = readPmxIndex(file, boneIndexSize);
    readBinary(file, b.additionalRate);
  }
  
  // Fixed Axis
  if (b.boneFlag & BoneFlag::FIXED_AXIS)
  {
    readBinary(file, b.axisVector);
  }
  
  // Local Axis
  if (b.boneFlag & BoneFlag::LOCAL_AXIS)
  {
    readBinary(file, b.xAxisVector);
    readBinary(file, b.zAxisVector);
  }
  
  // Key Value
  if (b.boneFlag & BoneFlag::EXTERNAL_PARENT_TRANSFORM)
  {
    readBinary(file, b.keyValue);
  }
  
  if (b.boneFlag & BoneFlag::INVERSE_KINEMATICS)
  {
    b.ikBoneIndex = readPmxIndex(file, boneIndexSize);
    readBinary(file, b.ikIteration);
    readBinary(file, b.ikLimitAngle);
    readBinary(file, b.ikLinkCount);
    
    // Each Link
    for (int i = 0; i < b.ikLinkCount; i++)
    {
      PMXIKBoneLink bl;
      
      bl.ikBoneIndex = readPmxIndex(file, boneIndexSize);
      readBinary(file, bl.enableAngleLimit);
      
      if (bl.enableAngleLimit != 0)
      {
        readBinary(file, bl.lowerLimit);
        readBinary(file, bl.upperLimit);
      }
      
      b.ikLinks.push_back(bl);
    // End for loop
    }
  // End if
  }
  return b;
}


PMXMorph PMXFile::readPMXMorph(std::ifstream &file, uint8_t morphIndexSize, uint8_t vertexIndexSize, uint8_t boneIndexSize, uint8_t materialIndexSize, uint8_t rigidIndexSize, uint8_t encoding)
{
  PMXMorph m;
  
  m.nameLocal = readPmxString(file, encoding);
  m.nameGlobal = readPmxString(file, encoding);
  
  readBinary(file, m.handlePanel);
  readBinary(file, m.morphType);
  readBinary(file, m.morphOffsetCount);
  
  for (int i = 0; i < m.morphOffsetCount; i++)
  {
    if (m.morphType == MorphType::GROUP)
    {
      PMXMorph::GroupMorph mg;
      
      mg.groupIndex = readPmxIndex(file, morphIndexSize);
      readBinary(file, mg.groupRate);
      
      m.groupMorph.push_back(mg);
    }
    
    else if (m.morphType == MorphType::FLIP)
    {
      PMXMorph::FlipMorph mf;
      
      mf.flipIndex = readPmxIndex(file, morphIndexSize);
      readBinary(file, mf.flipRate);
      
      m.flipMorph.push_back(mf);
    }
    
    else if (m.morphType == MorphType::VERTEX)
    {
      PMXMorph::VertexMorph mv;
      
      mv.vertexIndex = readPmxIndex(file, vertexIndexSize);
      readBinary(file, mv.positionOffset);
      
      m.vertexMorph.push_back(mv);
    }
    
    else if (m.morphType == MorphType::BONE)
    {
      PMXMorph::BoneMorph mb;
      
      mb.boneIndex = readPmxIndex(file, boneIndexSize);
      readBinary(file, mb.moveValue);
      readBinary(file, mb.rotationValue);
      
      m.boneMorph.push_back(mb);
    }
    
    else if (
      m.morphType == MorphType::UV 
      || m.morphType == MorphType::ADDITIONAL_UV1 
      || m.morphType == MorphType::ADDITIONAL_UV2 
      || m.morphType == MorphType::ADDITIONAL_UV3 
      || m.morphType == MorphType::ADDITIONAL_UV4)
    {
      PMXMorph::UVMorph muv;
      
      muv.vertexIndex = readPmxIndex(file, vertexIndexSize);
      readBinary(file, muv.uvOffset);
      
      m.uvMorph.push_back(muv);
    }
    
    else if (m.morphType == MorphType::MATERIAL)
    {
      PMXMorph::MaterialMorph mm;
      
      mm.materialIndex = readPmxIndex(file, materialIndexSize);
      readBinary(file, mm.offsetMethod);
      readBinary(file, mm.diffuseColor);
      readBinary(file, mm.specularColor);
      readBinary(file, mm.specularity);
      readBinary(file, mm.ambientColor);
      readBinary(file, mm.edgeColor);
      readBinary(file, mm.edgeSize);
      readBinary(file, mm.textureTint);
      readBinary(file, mm.environmentTint);
      readBinary(file, mm.toonTint);
      
      m.materialMorph.push_back(mm);
    }
    
    else if (m.morphType == MorphType::IMPULSE)
    {
      PMXMorph::ImpulseMorph mi;
      
      mi.rigidBodyIndex = readPmxIndex(file, rigidIndexSize);
      
      readBinary(file, mi.localFlag);
      readBinary(file, mi.moreVelocity);
      readBinary(file, mi.rotationTorque);
      
      m.impulseMorph.push_back(mi);
    }
    
    else
    {
      std::cerr << "Unknown morph type '" << static_cast<int>(m.morphType) << "'." << std::endl;
    }
  // End for loop
  }
  return m;
}


PMXFrame PMXFile::readPMXFrame(std::ifstream &file, uint8_t boneIndexSize, uint8_t morphIndexSize, uint8_t encoding)
{
  PMXFrame f;
  
  f.nameLocal = readPmxString(file, encoding);
  f.nameGlobal = readPmxString(file, encoding);
  
  readBinary(file, f.specialFrame);
  readBinary(file, f.elementCount);
  // std::cout << "Element count: " << f.elementCount << std::endl;
  
  for (int i = 0; i < f.elementCount; i++)
  {
    PMXFrame::Target ft;
    
    readBinary(file, ft.targetType);
    
    // Bone
    if (ft.targetType == PMXFrame::TargetType::BONE_INDEX)
    {
      ft.targetIndex = readPmxIndex(file, boneIndexSize);
    }
    // Morph
    else if (ft.targetType == PMXFrame::TargetType::MORPH_INDEX)
    {
      ft.targetIndex = readPmxIndex(file, morphIndexSize);
    }
    
    f.targets.push_back(ft);
  // End for loop
  }
  return f;
}


PMXRigidBody PMXFile::readPMXRigidBody(std::ifstream &file, uint8_t boneIndexSize, uint8_t encoding)
{
  PMXRigidBody rb;
  
  rb.nameLocal = readPmxString(file, encoding);
  rb.nameGlobal = readPmxString(file, encoding);
  
  rb.relatedBoneIndex = readPmxIndex(file, boneIndexSize);
  
  readBinary(file, rb.groupIndex);
  readBinary(file, rb.ignoreCollisionGroup);
  
  readBinary(file, rb.shapeType);
  readBinary(file, rb.shapeSize);
  
  readBinary(file, rb.colliderPosition);
  readBinary(file, rb.colliderRotation);
  
  readBinary(file, rb.weight);
  
  readBinary(file, rb.positionAttenuation);
  readBinary(file, rb.rotationAttenuation);
  
  readBinary(file, rb.recoil);
  readBinary(file, rb.friction);
  
  readBinary(file, rb.operationType);
  
  return rb;
}


PMXJoint PMXFile::readPMXJoint(std::ifstream &file, uint8_t rigidBodyIndexSize, uint8_t encoding)
{
  PMXJoint j;
  
  j.nameLocal = readPmxString(file, encoding);
  j.nameGlobal = readPmxString(file, encoding);
  
  readBinary(file, j.operationType);
  
  j.rigidBodyIndex1 = readPmxIndex(file, rigidBodyIndexSize);
  j.rigidBodyIndex2 = readPmxIndex(file, rigidBodyIndexSize);
  
  readBinary(file, j.position);
  readBinary(file, j.rotation);
  
  readBinary(file, j.positionConstraintLower);
  readBinary(file, j.positionConstraintUpper);
  
  readBinary(file, j.rotationConstraintLower);
  readBinary(file, j.rotationConstraintUpper);
  
  readBinary(file, j.springPosition);
  readBinary(file, j.springRotation);
  
  return j;
}


PMXFile::PMXFile(const char* filename)
{
  std::ifstream pmxFile{filename, std::ios::binary};
  if (!pmxFile)
  {
    std::cerr << "Failed to open file.\n";
    exit(1);
  }
  
  PMXHeader header;
  readBinary(pmxFile, header);
  
  PMXInfo info;
  readBinary(pmxFile, info);
  
  PMXIndexSize indexSize;
  readBinary(pmxFile, indexSize);
  
  localName = readPmxString(pmxFile, info.encoding);
  globalName = readPmxString(pmxFile, info.encoding);
  localComment = readPmxString(pmxFile, info.encoding);
  globalComment = readPmxString(pmxFile, info.encoding);
  
  // Vertex
  int32_t vertexCount = 0;
  readBinary(pmxFile, vertexCount);
  std::cout << "Vertex count: " << vertexCount << std::endl;
  
  for (int i = 0; i < vertexCount; i++)
  {
    vertices.push_back(readPmxVertex(pmxFile, info.additionalUV, indexSize.bone));
  }
  
  // Face Count
  int32_t faceCount = 0;
  readBinary(pmxFile, faceCount);
  std::cout << "Face count: " << static_cast<int>(faceCount) << std::endl;
  
  for (int i = 0; i < faceCount; i++)
  {
    int16_t index = 0;
    readBinary(pmxFile, index);
    indices.push_back(index);
  }
  
  // Texture
  std::filesystem::path directory = std::filesystem::path(filename).parent_path();
  int32_t textureCount = 0;
  readBinary(pmxFile, textureCount);
  std::cout << "Texture count: " << textureCount << std::endl;
  
  for (int i = 0; i < textureCount; i++)
  {
    std::string filename = readPmxString(pmxFile, info.encoding);
    std::filesystem::path path = directory / filename;
    std::string filepath = path.generic_string();
    std::cout << "Texture filepath: " << filepath << std::endl;
    // std::cout << "Texture filename: " << filename << std::endl;
    textures.push_back(filepath);
  }
  
  // Material
  int32_t materialCount = 0;
  readBinary(pmxFile, materialCount);
  std::cout << "Material count: " << materialCount << std::endl;
  
  for (int i = 0; i < materialCount; i++)
  {
    materials.push_back(readPMXMaterial(pmxFile, indexSize.texture, info.encoding));
  }
  
  // Bone
  int32_t boneCount = 0;
  readBinary(pmxFile, boneCount);
  std::cout << "Bone count: " << boneCount << std::endl;
  
  for (int i = 0; i < boneCount; i++)
  {
    bones.push_back(readPMXBone(pmxFile, indexSize.bone, info.encoding));
  }
  
  // Morph
  int32_t morphCount = 0;
  readBinary(pmxFile, morphCount);
  std::cout << "Morph count: " << morphCount << std::endl;
  
  for (int i = 0; i < morphCount; i++)
  {
    morphs.push_back(readPMXMorph(pmxFile, indexSize.morph, indexSize.vertex, indexSize.bone, indexSize.material, indexSize.rigid, info.encoding));
  }
  
  // Frame
  int32_t frameCount = 0;
  readBinary(pmxFile, frameCount);
  std::cout << "Frame count: " << static_cast<int>(frameCount) << std::endl;
  
  for (int i = 0; i < frameCount; i++)
  {
    frames.push_back(readPMXFrame(pmxFile, indexSize.bone, indexSize.morph, info.encoding));
  }
  
  // Rigid Body
  int32_t rigidBodyCount = 0;
  readBinary(pmxFile, rigidBodyCount);
  std::cout << "rigidBody count: " << static_cast<int>(rigidBodyCount) << std::endl;
  
  for (int i = 0; i < rigidBodyCount; i++)
  {
    rigidBodies.push_back(readPMXRigidBody(pmxFile, indexSize.bone, info.encoding));
  }
  
  // Read Joint
  int32_t jointCount = 0;
  readBinary(pmxFile, jointCount);
  std::cout << "Joint count: " << static_cast<int>(jointCount) << std::endl;
  
  for (int i = 0; i < jointCount; i++)
  {
    joints.push_back(readPMXJoint(pmxFile, indexSize.rigid, info.encoding));
  }
  
  // Compare read position and file size
  // std::streampos pos = pmxFile.tellg();
  // std::cout << "Current read position: " << pos << std::endl;
  
  // std::ifstream rePmxFile{filename, std::ios::binary | std::ios::ate};
  // if (!rePmxFile)
  // {
  //   std::cerr << "Failed to open file.\n";
  //   exit(1);
  // }
  // std::streampos fileSize = rePmxFile.tellg();
  // std::cout << "File size: " << fileSize << std::endl;
}