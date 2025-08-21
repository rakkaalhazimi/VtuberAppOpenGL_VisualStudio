#include "RotateBoneCommand.h"


RotateBoneCommand::RotateBoneCommand(PMXModel &model, int boneIndex, glm::vec3 &rotation): 
  model(model), boneIndex(boneIndex), newRotation(rotation)
{
  previousRotation = model.bones[boneIndex].rotation;
}

void RotateBoneCommand::execute()
{
  model.bones[boneIndex].rotation = newRotation;
}

void RotateBoneCommand::undo()
{
  model.bones[boneIndex].rotation = previousRotation;
}
