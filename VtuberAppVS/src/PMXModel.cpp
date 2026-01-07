#include "PMXModel.h"


PMXModel::PMXModel(PMXFile &pmxFile)
{
  for (size_t i = 0; i < pmxFile.bones.size(); i++)
  {
    int parent = pmxFile.bones[i].parentBoneIndex;
    boneChildren[parent].push_back(i);
    
    bool hasAddTranslation = (bool)(pmxFile.bones[i].boneFlag & BoneFlag::ADD_MOVEMENT);
    bool hasAddRotation = (bool)(pmxFile.bones[i].boneFlag & BoneFlag::ADD_ROTATION);

    bones.push_back(
      {
        pmxFile.bones[i].nameLocal,
        pmxFile.bones[i].nameGlobal,
        pmxFile.bones[i].parentBoneIndex,
        pmxFile.bones[i].additionalParentIndex,
        pmxFile.bones[i].additionalRate,
        pmxFile.bones[i].position,                                  // rest position
        glm::vec3(0.0f),                                            // position
        glm::vec3(0.0f),                                            // rotation
        hasAddTranslation,
        hasAddRotation,
      }
    );

    if (hasAddTranslation || hasAddRotation)
    {
      addParentIndexList.insert(pmxFile.bones[i].additionalParentIndex);
    }

    if (pmxFile.bones[i].ikBoneIndex > 0)
    {
      //auto currentBone = pmxFile.bones[i];
      //std::cout << "Current Bone index: " << i << std::endl;
      ////std::cout << "Current Bone Flag: " << currentBone.boneFlag << std::endl;
      //std::cout << "Additional Parent Index: " << currentBone.additionalParentIndex << std::endl;
      //std::cout << "Additional Rate: " << currentBone.additionalRate << std::endl;
      //std::cout << "Parent Index: " << currentBone.parentBoneIndex << std::endl;
      //std::cout << "Has add rotate: " << bones[i].hasAddRotation << std::endl;
      //std::cout << "Has add translate: " << bones[i].hasAddTranslation << std::endl;
      //std::cout << "End effector IK Bone index: " << currentBone.ikBoneIndex << std::endl;
      //std::cout << "Link count: " << currentBone.ikLinkCount << std::endl;
      //std::cout << "Link size: " << currentBone.ikLinks.size() << std::endl;

      //if (currentBone.ikLinks.size() < 1)
      //{
      //}
      //else
      //{
      //  for (size_t j = 0; j < currentBone.ikLinkCount; j++)
      //  {
      //    auto currentLink = currentBone.ikLinks[j];
      //    std::cout << "Bone linked index: " << currentLink.ikBoneIndex << std::endl;
      //  }
      //}
      //std::cout << std::endl;
    }

    // Add pmx original bones
    bonesPmx.push_back(pmxFile.bones[i]);
  }
  
  boneMatrices = std::vector<glm::mat4>(pmxFile.bones.size(), glm::mat4(1.0f));
  localTransform = boneMatrices;
  globalTransform = boneMatrices;

  addTranslation = std::vector<glm::vec3>(pmxFile.bones.size(), glm::vec3(0.0f));
  
  // Vertices
  // Convert vertices from PMXFile -> PMXModel  
  for (PMXVertex item: pmxFile.vertices)
  {
    
    glm::ivec4 boneIndicesVec(0);
    for (size_t i = 0; i < item.boneIndices.size() && i < 4; ++i) 
    {
        boneIndicesVec[i] = item.boneIndices[i];
    }
    
    glm::vec4 boneWeightsVec(0.0f);
    for (size_t i = 0; i < item.weights.size() && i < 4; ++i) 
    {
        boneWeightsVec[i] = item.weights[i];
    }

    vertices.push_back(
      VertexModel
      {
        item.position,
        item.normal,
        glm::vec2(item.uv.x, 1.0f - item.uv.y),
        boneIndicesVec,
        boneWeightsVec
      }
    );
    
  }
  
  baseVertices = vertices;
  // Clone vertices for skinning operation
  skinnedVertices = std::vector<VertexModel>(vertices);
  
  // Indices
  // CAVEAT: use the vertex index size (current size is 2 thus we use uint16_t)
  for (uint16_t item: pmxFile.indices)
  {
    indices.push_back(item);
  }
  
  // Textures
  for (int i = 0; i < pmxFile.textures.size(); i++)
  {
    textures.push_back(
      Texture{pmxFile.textures[i].c_str(), "myTexture", (GLuint)i}
    );
  }
  
  // Materials
  materials = pmxFile.materials;
  
  // Morphs
  morphs = pmxFile.morphs;
  
  // OpenGL Array Buffer
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  
  glBindVertexArray(VAO);
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexModel), vertices.data(), GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
  
  // Position (vec3)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)0);
  glEnableVertexAttribArray(0);
  
  // Normal (vec3)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  // UV (vec2)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  // Bone Indices (ivec4)
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexModel), (void*)(8 * sizeof(float)));
  glEnableVertexAttribArray(3);
  
  // Bone Weights (vec4)
  glVertexAttribPointer(4, 4, GL_INT, GL_FALSE, sizeof(VertexModel), (void*)(8 * sizeof(float) + 4 * sizeof(int)));
  glEnableVertexAttribArray(4);
  
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PMXModel::UpdateMorph(float &weight)
{
  // Wink right: ウィンク右
  // Wink left: ウィンク左
  // Wink: ウィンク
  // Wink: ウィンク２
  // float weight = 0.8;
  for (PMXMorph item: morphs)
  {
    if (item.nameLocal.find("ウィンク右") != std::string::npos)
    {
      // std::cout << "Morph name: " << item.nameLocal << std::endl;
      for (PMXMorph::VertexMorph vMorph: item.vertexMorph)
      {
        vertices[vMorph.vertexIndex].position =
          baseVertices[vMorph.vertexIndex].position + vMorph.positionOffset * weight;
      }
    }
  }
}

void PMXModel::Update()
{
  // Local Transform
  for (size_t i = 0; i < bones.size(); i++)
  {
    BoneModel bone = bones[i];
    localTransform[i] =
      glm::translate(glm::mat4(1.0f), bone.position) *
      glm::translate(glm::mat4(1.0f), bone.restPosition) *
      glm::toMat4(glm::quat(bone.rotation)) *
      glm::translate(glm::mat4(1.0f), -bone.restPosition);
  }

  // Additional Transform
  for (size_t i = 0; i < bones.size(); i++)
  {
    BoneModel bone = bones[i];
    glm::vec3 addTranslation(0.0f);
    glm::quat addRotation(1, 0, 0, 0);
    if (bone.hasAddTranslation)
    {
      addTranslation = localTransform[bone.addParentIndex][3] * bone.additionalRate;
    }
    if (bone.hasAddRotation)
    {
      glm::quat parentRotation = glm::quat_cast(glm::mat3(localTransform[bone.addParentIndex]));
      addRotation = glm::slerp(
        glm::quat(1, 0, 0, 0),
        parentRotation,
        bone.additionalRate
      );
    }
    localTransform[i] *=
      glm::translate(glm::mat4(1.0f), addTranslation) *
      glm::translate(glm::mat4(1.0f), bone.restPosition) *
      glm::toMat4(addRotation) *
      glm::translate(glm::mat4(1.0f), -bone.restPosition);
  }

  // Global Transform
  for (size_t i = 0; i < bones.size(); i++)
  {
    BoneModel bone = bones[i];
    if (bone.parentBoneIndex > 0)
    {
      globalTransform[i] = globalTransform[bone.parentBoneIndex] * localTransform[i];
    }
    else
    {
      globalTransform[i] = localTransform[i];
    }
  }

  boneMatrices = globalTransform;
}


void PMXModel::Draw(Shader &shader)
{
  shader.Activate();
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  
  for (size_t i = 0; i < skinnedVertices.size(); i++)
  {
    glm::vec4 skinnedPos = glm::vec4(0.0f);
    for (size_t j = 0; j < 4; j++)
    {
        int boneIndex = skinnedVertices[i].boneIndices[j];
        float weight = skinnedVertices[i].boneWeights[j];
        skinnedPos += weight * (boneMatrices[boneIndex] * glm::vec4(vertices[i].position, 1.0f));
    }
    skinnedVertices[i].position = skinnedPos;
  }
  
  glBufferSubData(GL_ARRAY_BUFFER, 0, skinnedVertices.size() * sizeof(VertexModel), skinnedVertices.data());
  
  int indexOffset = 0;
  
  for (int i = 0; i < materials.size(); i++)
  {
    int indexCount = materials[i].faceCount;
    int textureIndex = materials[i].textureIndex;
    
    textures[textureIndex].texUnit(shader, "myTexture", textureIndex);
    textures[textureIndex].Bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(indexOffset * sizeof(GLuint)));
    
    indexOffset += indexCount;
  }
}