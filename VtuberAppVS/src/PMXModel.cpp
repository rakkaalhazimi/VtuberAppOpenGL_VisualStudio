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
        glm::vec3(0.0f),                                            // addTranslation
        glm::quat(1, 0, 0, 0),                                      // addRotation
        glm::quat(1, 0, 0, 0),                                      // ikRotation
        glm::vec3(0.0f),                                            // ikPrevAngle
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
      auto currentBone = pmxFile.bones[i];

      if (currentBone.ikLinks.size() < 1)
      {
      }
      else
      {
        std::cout << "Current Bone index: " << i << std::endl;
      ////std::cout << "Current Bone Flag: " << currentBone.boneFlag << std::endl;
      //std::cout << "Additional Parent Index: " << currentBone.additionalParentIndex << std::endl;
      //std::cout << "Additional Rate: " << currentBone.additionalRate << std::endl;
      //std::cout << "Parent Index: " << currentBone.parentBoneIndex << std::endl;
      //std::cout << "Has add rotate: " << bones[i].hasAddRotation << std::endl;
      //std::cout << "Has add translate: " << bones[i].hasAddTranslation << std::endl;
        std::cout << "Bone position: " << currentBone.position.x << " " << currentBone.position.y << " " << currentBone.position.z << std::endl;
        std::cout << "End effector IK Bone index: " << currentBone.ikBoneIndex << std::endl;
        std::cout << "Link count: " << currentBone.ikLinkCount << std::endl;
        std::cout << "Link size: " << currentBone.ikLinks.size() << std::endl;
        for (size_t j = 0; j < currentBone.ikLinkCount; j++)
        {
          auto currentLink = currentBone.ikLinks[j];
          std::cout << "Bone linked index: " << currentLink.ikBoneIndex << std::endl;
        }
        std::cout << std::endl;
      }
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


// Get children and grandchildren of the bone
void PMXModel::GetBoneSubtree(int index, std::vector<int>& out)
{
  if (boneChildren.find(index) != boneChildren.end())
  {
    for (int childIndex : boneChildren[index])
    {
      //std::cout << "Child Index: " << childIndex << std::endl;
      out.push_back(childIndex);
      GetBoneSubtree(childIndex, out);
    }
  }
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


void PMXModel::UpdateLocalTransform(int index)
{
  BoneModel bone = bones[index];
  localTransform[index] =
    glm::translate(glm::mat4(1.0f), bone.position + bone.addTranslation) *
    glm::translate(glm::mat4(1.0f), bone.restPosition) *
    glm::toMat4(glm::quat(bone.ikRotation)) *
    glm::toMat4(glm::quat(bone.rotation)) *
    glm::toMat4(glm::quat(bone.addRotation)) *
    glm::translate(glm::mat4(1.0f), -bone.restPosition);
}


void PMXModel::UpdateIKTransform(int index)
  {
  BoneModel bone = bones[index];
  localTransform[index] =
    glm::translate(glm::mat4(1.0f), bone.position + bone.addTranslation) *
      glm::translate(glm::mat4(1.0f), bone.restPosition) *
    glm::toMat4(glm::quat(glm::vec3(3.0f, 0.0f, 3.0f))) *
      glm::toMat4(glm::quat(bone.rotation)) *
    glm::toMat4(glm::quat(bone.addRotation)) *
      glm::translate(glm::mat4(1.0f), -bone.restPosition);
  }


void PMXModel::UpdateAdditionalTransform(int index)
  {
  BoneModel bone = bones[index];
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
  bones[index].addTranslation = addTranslation;
  bones[index].addRotation = addRotation;
  UpdateLocalTransform(index);
  }


// Update the global transform of children and predecessors
void PMXModel::UpdateChildrenGlobalTransform(int index)
  {
  std::vector<int> chainIndex;
  GetBoneSubtree(index, chainIndex);
  for (int i : chainIndex)
  {
    //std::cout << "Current index: " << i << std::endl;
    UpdateAdditionalTransform(i);
    UpdateGlobalTransform(i);
  }
}


void PMXModel::UpdateGlobalTransform(int index)
{
  BoneModel bone = bones[index];
    if (bone.parentBoneIndex > 0)
    {
    globalTransform[index] = globalTransform[bone.parentBoneIndex] * localTransform[index];
    }
    else
    {
    globalTransform[index] = localTransform[index];
  }
}


void PMXModel::Update()
{
  // Local Transform
  for (size_t i = 0; i < bones.size(); i++)
  {
    UpdateLocalTransform(i);
  }

  // Additional and Global Transform (for Pre-IK)
  for (size_t i = 0; i < bones.size(); i++)
  {
    UpdateAdditionalTransform(i);
    UpdateGlobalTransform(i);
  }

  // IK Transform
  for (size_t i = 0; i < bones.size(); i++)
  {
    PMXBone pmxBone = bonesPmx[i];
    if (pmxBone.ikLinks.size() < 1)
    {
      continue;
    }

    int effectorIndex = pmxBone.ikBoneIndex;
    int targetIndex = i;
    glm::vec3 effector = GetBoneWorldPosition(effectorIndex);
    glm::vec3 target = GetBoneWorldPosition(targetIndex);

    // PROBLEM: with high iteration, the effector seems to back and forth between two specific values. 
    //          something might be wrong on how we update the joint.
    // Iteration: 1
    // Angle : 0.314159
    // Joint : : 0.819469 1.69394 0.743995
    // Effector : : 0.741878 - 0.651976 1.40664
    // Target : : 0.844506 10.2131 - 1.20874
    // 
    // Iteration : 2
    // Angle : 0.314159
    // Joint : : 0.819469 1.69394 0.743995
    // Effector : : 1.08322 - 0.729479 0.820845
    // Target : : 0.844506 10.2131 - 1.20874

    // DISCOVERY: The CCD Algorithm below is work, something might be wrong with how we update the delta rotation
    // DISCOVERY: Chat GPT said that I am using global axis to my local rotation, hence I need to make it local first


    // CCD Algorithm
    for (size_t j = 0; j < pmxBone.ikIteration; j++)
    {
      // Early stop when effector and target are close
      float dist = glm::distance2(effector, target);
      if (dist < 1e-3)
      {
        break;
      }

      // Rotate from Joint that is closer to Effector
      for (int k = 0; k < pmxBone.ikLinks.size(); k++)
      {
        effector = GetBoneWorldPosition(effectorIndex);
        //target = GetBoneWorldPosition(targetIndex);
        int jointIndex = pmxBone.ikLinks[k].ikBoneIndex;
        glm::vec3 joint = GetBoneWorldPosition(jointIndex);
        IK::axisAngle3D result = IK::solveAxisAngle3D(joint, effector, target);

        if (result.angle <= 1e-6 || glm::length(result.axis) <= 1e-6)
        {
          continue;
        }

        result.angle = glm::clamp(result.angle, -pmxBone.ikLimitAngle, pmxBone.ikLimitAngle);

        glm::quat globalRotation = glm::quat_cast(globalTransform[jointIndex]);
        glm::quat deltaRotation = glm::angleAxis(result.angle, result.axis);
        glm::quat chainRotation = glm::inverse(globalRotation) * deltaRotation * globalRotation;
        bones[jointIndex].ikRotation *= chainRotation;

        // Rotate all children of Joint
        for (int l = k - 1; l >= 0; --l)
        {
          int jointChildIndex = pmxBone.ikLinks[l].ikBoneIndex;
          bones[jointChildIndex].ikRotation *= chainRotation;
          UpdateLocalTransform(jointChildIndex);
          UpdateGlobalTransform(jointChildIndex);
          UpdateChildrenGlobalTransform(jointChildIndex);
        }
        UpdateLocalTransform(jointIndex);
        UpdateGlobalTransform(jointIndex);
        UpdateChildrenGlobalTransform(jointIndex);
        }

    }
    // End CCD Algorithm
    }
  // End IK Transform

  // Bone 84, 85, 86 are moved by additional transform
  // NOTE: to update single bone, you need to update all of its children
  // Global Transform (for Rendering)
  for (size_t i = 0; i < bones.size(); i++)
  {
    UpdateGlobalTransform(i);
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


glm::vec3 PMXModel::GetBoneWorldPosition(int index, bool isLog)
{
  glm::vec3 pos = globalTransform[index] * glm::vec4(bones[index].restPosition, 1.0f); // change target position
  //glm::vec3 pos = globalTransform[index][3];
  if (isLog)
  {
    std::cout << "Bone: " << index 
      << " position: " 
      << pos.x << " " 
      << pos.y << " " 
      << pos.z << std::endl;
  }
  return pos;
}


glm::vec3 PMXModel::GetBoneWorldDist(int indexA, int indexB, bool isLog)
{
  glm::vec3 posA = GetBoneWorldPosition(indexA);
  glm::vec3 posB = GetBoneWorldPosition(indexB);
  float dist = glm::distance2(posA, posB);
  //glm::vec3 pos = bones[index].restPosition;
  if (isLog)
  {
    std::cout << "Bone: " << indexA << " " << indexB
      << " distance: " << dist << std::endl;
  }
  return posA;
}


float PMXModel::GetIKChainLength(int targetIndex)
{
  float chainLength = 0.0f;
  int effectorIndex = bonesPmx[targetIndex].ikBoneIndex;
  int prev = effectorIndex;
  for (const auto& link : bonesPmx[targetIndex].ikLinks)
  {
    int curr = link.ikBoneIndex;
    chainLength += glm::distance(
      GetBoneWorldPosition(prev),
      GetBoneWorldPosition(curr)
    );
    prev = curr;
  }
  return chainLength;
}


float PMXModel::GetIKRelativeChainLength(int targetIndex)
{
  float chainLength = 0.0f;
  int prev = targetIndex;
  for (const auto& link : bonesPmx[targetIndex].ikLinks)
  {
    int curr = link.ikBoneIndex;
    chainLength += glm::distance(
      GetBoneWorldPosition(prev),
      GetBoneWorldPosition(curr)
    );
    prev = curr;
  }
  return chainLength;
}


glm::quat PMXModel::GetParentBoneWorldRot(int index, bool isLog)
{
  BoneModel bone = bones[index];
  glm::quat parentRot = glm::quat_cast(globalTransform[index]);
  glm::vec3 euler = glm::eulerAngles(parentRot);
  if (isLog)
  {
    std::cout << "index: " << index << std::endl;
    Utils::printVector(euler, "Parent world rotation");
  }
  return parentRot;
}