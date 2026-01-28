#include "commands/TranslateBoneCommand.h"


TranslateBoneCommand::TranslateBoneCommand(PMXModel& model, int boneIndex, glm::vec3& position) :
  model(model), boneIndex(boneIndex), newPosition(position)
{
  previousPosition = model.bones[boneIndex].position;
}

void TranslateBoneCommand::execute()
{
  model.bones[boneIndex].position = newPosition;
}

void TranslateBoneCommand::undo()
{
  model.bones[boneIndex].position = previousPosition;
}