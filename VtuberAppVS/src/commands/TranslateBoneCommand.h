#pragma once

#include "PMXModel.h"
#include "commands/Command.h"


class TranslateBoneCommand : public Command
{
private:
  PMXModel& model;
  int boneIndex;
  glm::vec3& newPosition;
  glm::vec3 previousPosition;

public:
  TranslateBoneCommand(PMXModel& model, int boneIndex, glm::vec3& position);

  void execute() override;
  void undo() override;
};