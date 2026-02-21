#include "RotateBoneCommand.h"


RotateBoneCommand::RotateBoneCommand(PMXModel &model, int boneIndex, glm::vec3 &rotation): 
  model(model), boneIndex(boneIndex), newRotation(rotation)
{
  previousRotation = model.bones[boneIndex].getRotation();
}

void RotateBoneCommand::execute()
{
  model.bones[boneIndex].setRotation(newRotation);
}

void RotateBoneCommand::undo()
{
  model.bones[boneIndex].setRotation(previousRotation);
}
