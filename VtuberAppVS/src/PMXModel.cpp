#include "PMXModel.h"


void BoneModel::setRotation(glm::vec3 &eulers)
{
  rotation = eulers;
  quadRotation = glm::quat(eulers);
}

void BoneModel::setQuadRotation(glm::quat &q)
{
  rotation = glm::eulerAngles(q);
  quadRotation = q;
}

glm::quat BoneModel::getQuadRotation() const
{
  return quadRotation;
}

glm::vec3 BoneModel::getRotation() const
{
  return rotation;
}


PMXModel::PMXModel(PMXFile &pmxFile)
{
  for (size_t i = 0; i < pmxFile.bones.size(); i++)
  {
    int parent = pmxFile.bones[i].parentBoneIndex;
    boneChildren[parent].push_back(i);
    boneNameToIndex[pmxFile.bones[i].nameLocal] = i;
    
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
        glm::quat(1, 0, 0, 0),                                      // quadRotation
        glm::vec3(0.0f),                                            // addTranslation
        glm::quat(1, 0, 0, 0),                                      // addRotation
        glm::quat(1, 0, 0, 0),                                      // ikRotation
        glm::vec3(0.0f),                                            // ikPrevAngle
        hasAddTranslation,
        hasAddRotation,
      }
    );

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
  for (auto& item : morphs)
  {
    morphWeights[item.nameLocal.c_str()] = 0.0f;
    //std::cout << "Morph name: " << item.nameLocal << std::endl;
  }

  // Rigid Body
  rigidBodyPmx = pmxFile.rigidBodies;

  // Joint
  jointsPmx = pmxFile.joints;

  // Physics
  for (size_t i = 0; i < bones.size(); i++)
  {
    UpdateLocalTransform(i);
  }
  for (size_t i = 0; i < bones.size(); i++)
  {
    UpdateAdditionalTransform(i);
    UpdateGlobalTransform(i);
  }
  InitPhysics();
  
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


void PMXModel::CreateRigidBody()
{
  for (auto& item : rigidBodyPmx)
  {
    if (item.nameLocal.find("耳") == std::string::npos)
    {
      //continue;
      //std::cout << "Found ears" << std::endl;
    }
    else
    {
      //std::cout << "Bone Index ears: " << item.relatedBoneIndex << std::endl;
      auto& bone = bonesPmx[item.relatedBoneIndex];
      //std::cout << "Bone flag: " << bone.boneFlag << std::endl;
    }


    if (item.operationType == PMXRigidBody::OperationType::STATIC)
    {
      //std::cout << "Bone Operation type: " << "static" << std::endl;
    }
    else if (item.operationType == PMXRigidBody::OperationType::DYNAMIC)
    {
      //std::cout << "Bone Operation type: " << "dynamic" << std::endl;
    }
    else if (item.operationType == PMXRigidBody::OperationType::DYNAMIC_POSITION_ADJUST)
    {
      //std::cout << "Bone Operation type: " << "dynamic merge" << std::endl;
    }

    // Shape
    btCollisionShape* shape{};

    switch (item.shapeType)
    {
    case PMXRigidBody::ShapeType::SPHERE:
      shape = new btSphereShape(item.shapeSize.x);
      //std::cout << "SPHERE" << std::endl;
      break;
    case PMXRigidBody::ShapeType::BOX:
      //std::cout << "BOX" << std::endl;
      shape = new btBoxShape(
        btVector3(item.shapeSize.x, item.shapeSize.y, item.shapeSize.z));
      break;
    case PMXRigidBody::ShapeType::CAPSULE:
      //std::cout << "CAPSULE" << std::endl;
      shape = new btCapsuleShape(item.shapeSize.x, item.shapeSize.y);
      break;
    default:
      continue;
      break;
    }

    // Collider
    btTransform colliderOffset;
    colliderOffset.setIdentity();
    colliderOffset.setOrigin(
      btVector3(
        item.colliderPosition.x,
        item.colliderPosition.y,
        item.colliderPosition.z)
    );

    btQuaternion q;
    q.setEuler(
      item.colliderRotation.y,
      item.colliderRotation.x,
      item.colliderRotation.z
    );
    colliderOffset.setRotation(q);

    //Utils::printVector(item.colliderPosition, "Collider: ");

    btTransform boneGlobalTransform;
    boneGlobalTransform.setFromOpenGLMatrix(
      glm::value_ptr(globalTransform[item.relatedBoneIndex])
    );
    //colliderOffset = boneGlobalTransform * colliderOffset;

    // Mass
    btScalar mass = 0.0f;
    btVector3 inertia(0, 0, 0);

    // Temporarily change to static operation type
    //item.operationType = PMXRigidBody::OperationType::STATIC;

    if (item.operationType != PMXRigidBody::OperationType::STATIC)
    {
      mass = item.weight;
    }
    if (mass != 0.0f)
    {
      shape->calculateLocalInertia(mass, inertia);
    }
    //std::cout << "Mass: " << mass << std::endl;

    // Motion for interpolation
    btDefaultMotionState* motion =
      //new btDefaultMotionState(btTransform::getIdentity());
      new btDefaultMotionState(colliderOffset);

    btRigidBody::btRigidBodyConstructionInfo info(
      mass,
      motion,
      shape,
      inertia
    );
    info.m_linearDamping = item.positionAttenuation;
    info.m_angularDamping = item.rotationAttenuation;
    info.m_restitution = item.recoil;
    info.m_friction = item.friction;
    info.m_additionalDamping = true;

    // Rigid Body
    btRigidBody* body = new btRigidBody(info);

    // Prevent the rigid body from going to sleep.
    body->setActivationState(DISABLE_DEACTIVATION);

    // Enable Kinematic Behaviors:
    // - Doesn't affected by physics.
    // - Solid obstacle to dynamic object.
    // - Moved by our script/code.
    if (item.operationType == PMXRigidBody::OperationType::STATIC)
    {
      body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
    }

    int collisionGroup = 1 << item.groupIndex;
    int collisionMask = item.ignoreCollisionGroup;

    physWorld->addRigidBody(body, collisionGroup, collisionMask);

    rigidBody.push_back(
      RigidBodyModel
      {
        colliderOffset,
        shape,
        body,
        item.operationType,
        item.relatedBoneIndex,
      });
  }
}


void PMXModel::CreateJoints()
{
  for (const auto& item : jointsPmx)
  {
    // 1. Create the Joint's World Transform from PMX data
    btTransform jointWorld;
    jointWorld.setIdentity();
    jointWorld.setOrigin(btVector3(item.position.x, item.position.y, item.position.z));

    btQuaternion q;
    q.setEuler(item.rotation.y, item.rotation.x, item.rotation.z); // Using PMX Heading/Pitch/Bank
    jointWorld.setRotation(q);

    // 2. Get the World Transforms of the two Rigid Bodies
    // These should be the ones you calculated during the Rigid Body init
    btRigidBody* bodyA = rigidBody[item.rigidBodyIndex1].body;
    btRigidBody* bodyB = rigidBody[item.rigidBodyIndex2].body;

    btTransform frameInA = bodyA->getWorldTransform().inverse() * jointWorld;
    btTransform frameInB = bodyB->getWorldTransform().inverse() * jointWorld;

    btGeneric6DofSpringConstraint* joint = new btGeneric6DofSpringConstraint(
      *bodyA, *bodyB, frameInA, frameInB, true
    );

    // Position Limits
    joint->setLinearLowerLimit(
      btVector3
      (
        item.positionConstraintLower.x,
        item.positionConstraintLower.y,
        item.positionConstraintLower.z
      )
    );
    joint->setLinearUpperLimit(
      btVector3
      (
        item.positionConstraintUpper.x,
        item.positionConstraintUpper.y,
        item.positionConstraintUpper.z
      )
    );

    // Rotation Limits (Radians)
    joint->setAngularLowerLimit(
      btVector3
      (
        item.rotationConstraintLower.x,
        item.rotationConstraintLower.y,
        item.rotationConstraintLower.z
      )
    );
    joint->setAngularUpperLimit(
      btVector3
      (
        item.rotationConstraintUpper.x,
        item.rotationConstraintUpper.y,
        item.rotationConstraintUpper.z
      )
    );

    // Spring
    if (item.springPosition.x != 0)
    {
      joint->enableSpring(0, true);
      joint->setStiffness(0, item.springPosition.x);
    }
    if (item.springPosition.y != 0)
    {
      joint->enableSpring(1, true);
      joint->setStiffness(1, item.springPosition.y);
    }
    if (item.springPosition.z != 0)
    {
      joint->enableSpring(2, true);
      joint->setStiffness(2, item.springPosition.z);
    }
    if (item.springRotation.x != 0)
    {
      joint->enableSpring(3, true);
      joint->setStiffness(3, item.springRotation.x);
    }
    if (item.springRotation.y != 0)
    {
      joint->enableSpring(4, true);
      joint->setStiffness(4, item.springRotation.y);
    }
    if (item.springRotation.z != 0)
    {
      joint->enableSpring(5, true);
      joint->setStiffness(5, item.springRotation.z);
    }

    // true = bodies linked by joint won't collide with each other
    physWorld->addConstraint(joint, true);
  }
}


void PMXModel::InitPhysics()
{
  // World
  physBroadphase = new btDbvtBroadphase();
  physConfig = new btDefaultCollisionConfiguration();
  physDispatcher = new btCollisionDispatcher(physConfig);
  physSolver = new btSequentialImpulseConstraintSolver();
  physWorld = new btDiscreteDynamicsWorld(physDispatcher, physBroadphase, physSolver, physConfig);

  //physWorld->setGravity(btVector3(0, -9.8f * 10.0f, 0));
  btStaticPlaneShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0.0f);

  btTransform groundTransform;
  groundTransform.setIdentity();
  btDefaultMotionState* groundMS = new btDefaultMotionState(groundTransform);

  btRigidBody::btRigidBodyConstructionInfo groundInfo(0, groundMS, groundShape, btVector3(0, 0, 0));
  btRigidBody groundRB(groundInfo);

  CreateRigidBody();
  CreateJoints();
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


void PMXModel::UpdateMorph(const char *name, float &weight)
{
  // Wink right: ウィンク右
  // Wink left: ウィンク左
  // Wink: ウィンク
  // Wink: ウィンク２
  // float weight = 0.8;
  for (PMXMorph item: morphs)
  {
    
    switch (item.morphType)
    {
      case MorphType::VERTEX:
        if (item.nameLocal.find(name) != std::string::npos)
        {
          for (PMXMorph::VertexMorph vMorph : item.vertexMorph)
          {
            vertices[vMorph.vertexIndex].position =
              baseVertices[vMorph.vertexIndex].position + vMorph.positionOffset * weight;
          }
        }
        break;

      default:
        break;
       
    }
  }
}


void PMXModel::UpdatePhysics()
{
  if (physWorld == nullptr)
  {
    return;
  }

  physWorld->stepSimulation(1.0f / 60.0f);

  //for (const auto& item : rigidBody)
  //{
  //  // For each Dynamic rigid body:
  //  btTransform boneWorldTrans;
  //  boneWorldTrans.setFromOpenGLMatrix(glm::value_ptr(globalTransform[item.relatedBoneIndex]));
  //  btTransform targetWorldTrans = boneWorldTrans * item.colliderOffset;

  //  btVector3 targetPos = targetWorldTrans.getOrigin();
  //  btVector3 currentPos = item.body->getWorldTransform().getOrigin();

  //  // Calculate a simple spring force: Force = stiffness * (target - current)
  //  btVector3 force = (targetPos - currentPos) * 100.0f;
  //  item.body->applyCentralForce(force);

  //  // Add some air resistance so it doesn't vibrate forever
  //  item.body->setLinearVelocity(item.body->getLinearVelocity() * 0.95f);
  //}

  // Global Transform after physics
  std::vector<int32_t> included = {172, 173, 174, 175, 177, 178, 179, 180};
  
  for (const auto& item : rigidBody)
  {
    auto it = std::find(included.begin(), included.end(), item.relatedBoneIndex);
    if (it == included.end())
    {
      //continue;
    }

    // Static
    if (item.operationType == PMXRigidBody::OperationType::STATIC)
    {
      btTransform boneWorldTrans;
      boneWorldTrans.setFromOpenGLMatrix(
        glm::value_ptr(globalTransform[item.relatedBoneIndex])
      );
      btTransform physicsWorldTrans = boneWorldTrans * item.colliderOffset;
      item.body->getMotionState()->setWorldTransform(physicsWorldTrans);
    }

    // Dynamic and DynamicMerge
    if (item.operationType != PMXRigidBody::OperationType::STATIC)
    {
      btTransform physicsWorldTrans;
      item.body->getMotionState()->getWorldTransform(physicsWorldTrans);

      // Reverse the offset to find where the BONE should be
      btTransform boneWorldTrans = physicsWorldTrans * item.colliderOffset.inverse();

      // Update your PMX Bone with this new matrix for rendering
      btScalar matrixArray[16];
      boneWorldTrans.getOpenGLMatrix(matrixArray);
      glm::mat4 btGlobalTransform = glm::make_mat4(matrixArray);
      //glm::vec3 boneWorldPos = GetBoneWorldPosition(item.relatedBoneIndex);
      glm::vec3 boneWorldPhysPos = glm::vec3(btGlobalTransform[3]);
      glm::vec3 boneWorldPos = glm::vec3(globalTransform[item.relatedBoneIndex][3]);

      //btGlobalTransform[3] = globalTransform[item.relatedBoneIndex][3];
      globalTransform[item.relatedBoneIndex] = btGlobalTransform;
      //UpdateChildrenGlobalTransform(item.relatedBoneIndex);
    }
  }
  //hasPrint = true;

  // Physics live in global world, change it to local world
  /*for (const auto& item : rigidBody)
  {
    BoneModel bone = bones[item.relatedBoneIndex];
    if (bone.parentBoneIndex > 0)
    {
      localTransform[item.relatedBoneIndex] = 
        glm::inverse(globalTransform[bone.parentBoneIndex]) *
        globalTransform[item.relatedBoneIndex];
    }
    else
    {
      localTransform[item.relatedBoneIndex] = globalTransform[item.relatedBoneIndex];
    }
  }*/

  for (size_t i = 0; i < bones.size(); i++)
  {
    //UpdateGlobalTransform(i);
  }
}


void PMXModel::UpdateLocalTransform(int index)
{
  BoneModel bone = bones[index];
  localTransform[index] =
    glm::translate(glm::mat4(1.0f), bone.position + bone.addTranslation) *
    glm::translate(glm::mat4(1.0f), bone.restPosition) *
    glm::toMat4(glm::quat(bone.ikRotation)) *
    glm::toMat4(bone.getQuadRotation()) *
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

        // Axis limit is how far the bone can rotate.
        // Angle limit is step size of rotation.
        // 
        // Rotate Right Knee Manually
        // Rotate x-positive knee bend forward
        // Rotate x-negative knee bend backward
        //
        // IK with x-axis
        // Position-y negative or positive => negative angle and axis (1 0 0) didn't move
        // 
        // IK normal
        // Position-y positive => positive angle and negative axis, move leg up
        // Position-y negative => positive angle and positive axis, move leg down
        IK::axisAngle3D result;
        if (pmxBone.ikLinks[k].enableAngleLimit)
        {
          if ((pmxBone.ikLinks[k].lowerLimit.x != 0 || pmxBone.ikLinks[k].upperLimit.x != 0) &&
            (pmxBone.ikLinks[k].lowerLimit.y == 0 || pmxBone.ikLinks[k].upperLimit.y == 0) &&
            (pmxBone.ikLinks[k].lowerLimit.z == 0 || pmxBone.ikLinks[k].upperLimit.z == 0)
            )
          {
            glm::vec3 axis(1, 0, 0);
            result = IK::solveSingleAxisAngle2D(axis, joint, effector, target);
            //std::cout << "Angle X-Axis: " << result.angle << std::endl;
            //Utils::printVector(result.axis, "Axis");
          }
          else if ((pmxBone.ikLinks[k].lowerLimit.x == 0 || pmxBone.ikLinks[k].upperLimit.x == 0) &&
            (pmxBone.ikLinks[k].lowerLimit.y != 0 || pmxBone.ikLinks[k].upperLimit.y != 0) &&
            (pmxBone.ikLinks[k].lowerLimit.z == 0 || pmxBone.ikLinks[k].upperLimit.z == 0)
            )
          {
            glm::vec3 axis(0, 1, 0);
            result = IK::solveSingleAxisAngle2D(axis, joint, effector, target);
          }
          else if ((pmxBone.ikLinks[k].lowerLimit.x == 0 || pmxBone.ikLinks[k].upperLimit.x == 0) &&
            (pmxBone.ikLinks[k].lowerLimit.y == 0 || pmxBone.ikLinks[k].upperLimit.y == 0) &&
            (pmxBone.ikLinks[k].lowerLimit.z != 0 || pmxBone.ikLinks[k].upperLimit.z != 0)
            )
          {
            glm::vec3 axis(0, 0, 1);
            result = IK::solveSingleAxisAngle2D(axis, joint, effector, target);
          }
          else
          {
            result = IK::solveAxisAngle3D(joint, effector, target);
          }
        }
        else
        {
          result = IK::solveAxisAngle3D(joint, effector, target);
        }

        //result = IK::solveAxisAngle3D(joint, effector, target);

        if (result.angle <= 1e-6 || glm::length(result.axis) <= 1e-6)
        {
          continue;
        }

        result.angle = glm::clamp(result.angle, -pmxBone.ikLimitAngle, pmxBone.ikLimitAngle);

        glm::quat globalRotation = glm::quat_cast(globalTransform[jointIndex]);
        glm::quat deltaRotation = glm::angleAxis(result.angle, result.axis);

        if (pmxBone.ikLinks[k].enableAngleLimit && (jointIndex == 85))
        {
          glm::vec3 currentEuler = bones[jointIndex].ikPrevAngle;
          glm::vec3 originalEuler = glm::eulerAngles(bones[jointIndex].ikRotation);
          glm::vec3 deltaEuler = glm::eulerAngles(deltaRotation);
          glm::vec3 predictedEuler = currentEuler + deltaEuler;
          glm::vec3 limitedEuler = 
            glm::clamp(
              predictedEuler, 
              pmxBone.ikLinks[k].lowerLimit, 
              pmxBone.ikLinks[k].upperLimit
          );
          glm::vec3 allowedEuler = 
            glm::clamp(
              limitedEuler - currentEuler, 
              -pmxBone.ikLimitAngle, 
              pmxBone.ikLimitAngle
           );

          deltaRotation = glm::quat(allowedEuler);
          bones[jointIndex].ikPrevAngle += allowedEuler;
        }

        glm::quat chainRotation = glm::inverse(globalRotation) * deltaRotation * globalRotation;
        bones[jointIndex].ikRotation *= chainRotation;

        // Rotate all children of Joint
        for (int l = k - 1; l >= 0; --l)
        {
          // Disable child rotation so the child doesn't have
          // euler starting point.
          //int jointChildIndex = pmxBone.ikLinks[l].ikBoneIndex;
          //bones[jointChildIndex].ikRotation *= chainRotation;
          //UpdateLocalTransform(jointChildIndex);
          //UpdateGlobalTransform(jointChildIndex);
          //UpdateChildrenGlobalTransform(jointChildIndex);
        }
        UpdateLocalTransform(jointIndex);
        UpdateGlobalTransform(jointIndex);
        UpdateChildrenGlobalTransform(jointIndex);
      }

    }
    // End CCD Algorithm
  }
  // End IK Transform

  // Test Physics
  //UpdatePhysics();

  // Global Transform (for Rendering)
  for (size_t i = 0; i < bones.size(); i++)
  {
    UpdateGlobalTransform(i);
  }

  UpdatePhysics();

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
  }*/

  //for (int i = 0; i < materials.size(); i++)
  for (PMXMaterial &material : materials)
  {
    int indexCount = material.faceCount;
    int textureIndex = material.textureIndex;
    int environmentIndex = material.environmentIndex;

    textures[textureIndex].texUnit(shader, "myTexture", material.textureIndex);
    textures[textureIndex].Bind();

    if (material.environmentMode > 0 && material.environmentIndex != -1)
    {
      textures[environmentIndex].texUnit(shader, "envTexture", environmentIndex);
      textures[environmentIndex].Bind();
    }

    glUniform1i(
      glGetUniformLocation(shader.ID, "envMode"),
      material.environmentMode
    );

    glUniform4f(
      glGetUniformLocation(shader.ID, "diffuseColor"), 
      material.diffuseColor.x, material.diffuseColor.y, material.diffuseColor.z, material.diffuseColor.w
    );

    glUniform3f(
      glGetUniformLocation(shader.ID, "ambientColor"),
      material.ambientColor.x, material.ambientColor.y, material.ambientColor.z
    );

    glUniform3f(
      glGetUniformLocation(shader.ID, "specularColor"),
      material.specularColor.x, material.specularColor.y, material.specularColor.z
    );

    glUniform1f(
      glGetUniformLocation(shader.ID, "shininess"),
      material.specularity
    );



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
