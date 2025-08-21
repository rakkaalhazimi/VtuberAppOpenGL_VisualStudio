#pragma once

#include<glm/gtc/matrix_transform.hpp>
#include<imgui/imgui.h>
#include<imgui/imgui_impl_glfw.h>
#include<imgui/imgui_impl_opengl3.h>

#include "commands/CommandManager.h"
#include "commands/RotateBoneCommand.h"
#include "PMXModel.h"

class PMXEditorGUI
{
  private:
    PMXModel &model;
    CommandManager &commandManager;
    std::vector<int> boneIndices = { 16, 55, 60 };
    
  public:
    PMXEditorGUI(PMXModel &model, CommandManager &commandManager);
    void draw();
};