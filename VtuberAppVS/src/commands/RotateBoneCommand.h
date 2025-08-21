#pragma once

#include "PMXModel.h"
#include "commands/Command.h"


class RotateBoneCommand: public Command
{
  private:
    PMXModel &model;
    int boneIndex;
    glm::vec3 &newRotation;
    glm::vec3 previousRotation;
      
  public:
    RotateBoneCommand(PMXModel &model, int boneIndex, glm::vec3 &rotation);
    
    void execute() override;
    void undo() override;
};