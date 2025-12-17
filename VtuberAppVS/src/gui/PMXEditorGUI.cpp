#include "gui/PMXEditorGUI.h"


PMXEditorGUI::PMXEditorGUI(PMXModel &model, CommandManager &cmdManager): 
  model(model), commandManager(commandManager) 
{
  std::string content;
  std::ifstream infile("assets/text/bones-common-rotate.txt");
  while (std::getline(infile, content)) 
  {
    auto pos = content.find(' ');
    if (pos != std::string::npos)
    {
      std::string localName = content.substr(0, pos);
      std::string globalName = content.substr(pos + 1);
      boneMap[localName] = globalName;
    }
  }
  /*for (size_t i = 0; i < model.bonesPmx.size(); i++)
  {
    PMXBone currentBonePMX = model.bonesPmx[i];
    std::cout << currentBonePMX.nameLocal << std::endl;
  }*/
}

void PMXEditorGUI::draw()
{
  
  glm::vec3 boneRotation(0.0f);
  
  ImGui::Begin("MMD Model");
    
  if (ImGui::CollapsingHeader("Bones"))
  {
    for (size_t i = 0; i < model.bonesPmx.size(); i++)
    {
      PMXBone currentBonePMX = model.bonesPmx[i];
      // Skip non-common bones
      auto it = boneMap.find(currentBonePMX.nameLocal);
      if (it == boneMap.end())
      {
        continue;
      }

      BoneModel currentBone = model.bones[i];
      glm::vec3 currentBoneRotation = currentBone.rotation;
      std::string showedBoneName = boneMap[currentBonePMX.nameLocal];
      
      if (ImGui::TreeNode(showedBoneName.c_str()))
      {
        // previousRotation = boneRotation;
        bool isSliderXActive = ImGui::SliderFloat("rotation-x", &currentBoneRotation.x, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        ImGui::SameLine();
        ImGui::InputFloat("##rotation-x", &currentBoneRotation.x, 0.01f, 0.1f, "%.3f");
        bool isSliderYActive = ImGui::SliderFloat("rotation-y", &currentBoneRotation.y, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        bool isSliderZActive = ImGui::SliderFloat("rotation-z", &currentBoneRotation.z, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        
        if ((isSliderXActive || isSliderYActive || isSliderZActive))
        {
          // isRotating = true;
          // command.execute();
          RotateBoneCommand command(model, (int)i, currentBoneRotation);
          command.execute();
        }
        else
        {
          // isRotating = false;
        }
        
        // bool isEqual = glm::all(glm::epsilonEqual(boneRotation, previousRotation, 1e-8f));
        // if (!isEqual)
        // {
          // std::cout << "Finish Rotating" << std::endl;
          // commandManager.executeCommand(
          //   std::make_unique<RotateBoneCommand>(command)
          // );
        // }
        ImGui::TreePop();
      }
    }
    
    
    
    
  }
  
  if (ImGui::CollapsingHeader("Morphs"))
  {
    // ImGui::SliderFloat("slider float", &morphWeight, 0.0f, 1.0f, "ratio = %.3f");
  }
  
  ImGui::MenuItem("(demo menu)", NULL, false, false);
  ImGui::End();
}