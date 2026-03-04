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
      glm::vec3 currentBonePosition = currentBone.position;
      std::string showedBoneName = boneMap[currentBonePMX.nameLocal];
      
      if (ImGui::TreeNode(showedBoneName.c_str()))
      {
        // previousRotation = boneRotation;
        bool isSliderXActive = ImGui::SliderFloat("rotation-x", &currentBoneRotation.x, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        ImGui::SameLine();
        bool isInputXActive = ImGui::InputFloat("##rotation-x", &currentBoneRotation.x, 0.01f, 0.1f, "%.3f");

        bool isSliderYActive = ImGui::SliderFloat("rotation-y", &currentBoneRotation.y, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        ImGui::SameLine();
        bool isInputYActive = ImGui::InputFloat("##rotation-y", &currentBoneRotation.y, 0.01f, 0.1f, "%.3f");

        bool isSliderZActive = ImGui::SliderFloat("rotation-z", &currentBoneRotation.z, -glm::pi<float>(), glm::pi<float>(), "%.3f");
        ImGui::SameLine();
        bool isInputZActive = ImGui::InputFloat("##rotation-z", &currentBoneRotation.z, 0.01f, 0.1f, "%.3f");
        
        if (
          isSliderXActive || isSliderYActive || isSliderZActive || 
          isInputXActive || isInputYActive || isInputZActive
          )
        {
          RotateBoneCommand command(model, (int)i, currentBoneRotation);
          command.execute();
        }

        bool isPositionXActive = ImGui::SliderFloat("position-x", &currentBonePosition.x, -10.0f, 10.0f, "%.3f");
        ImGui::SameLine();
        bool isInputPositionXActive = ImGui::InputFloat("##position-x", &currentBonePosition.x, 0.01f, 0.1f, "%.3f");

        bool isPositionYActive = ImGui::SliderFloat("position-y", &currentBonePosition.y, -10.0f, 10.0f, "%.3f");
        ImGui::SameLine();
        bool isInputPositionYActive = ImGui::InputFloat("##position-y", &currentBonePosition.y, 0.01f, 0.1f, "%.3f");

        bool isPositionZActive = ImGui::SliderFloat("position-z", &currentBonePosition.z, -10.0f, 10.0f, "%.3f");
        ImGui::SameLine();
        bool isInputPositionZActive = ImGui::InputFloat("##position-z", &currentBonePosition.z, 0.01f, 0.1f, "%.3f");

        if (
          isPositionXActive || isPositionYActive || isPositionZActive ||
          isInputPositionXActive || isInputPositionYActive || isInputPositionZActive
          )
        {
          TranslateBoneCommand command(model, (int)i, currentBonePosition);
          command.execute();
        }

        ImGui::TreePop();
      }
    }
    
  }
  
  if (ImGui::CollapsingHeader("Morphs"))
  {
    int id = 0;

    for (auto &item : model.morphs)
    {
      switch (item.morphType)
      {
        case (MorphType::VERTEX):
          ImGui::PushID(id);
          float currentWeight = model.morphWeights[item.nameLocal.c_str()];
          bool isSliderActive = ImGui::SliderFloat(item.nameLocal.c_str(), &currentWeight, 0.0f, 1.0f, "%.2f");
          if (isSliderActive)
          {
            model.UpdateMorph(item.nameLocal.c_str(), currentWeight);
            model.morphWeights[item.nameLocal.c_str()] = currentWeight;
          }
          ImGui::PopID();
          id++;
          break;

      }

    }

    // ImGui::SliderFloat("slider float", &morphWeight, 0.0f, 1.0f, "ratio = %.3f");
  }
  
  ImGui::MenuItem("(demo menu)", NULL, false, false);
  ImGui::End();
}
