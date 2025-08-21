#include "Selector.h"


Selector::Selector()
{
};


void Selector::Watch(GLFWwindow* window, RayCaster& rayCaster, std::vector<Mesh*> meshes)
{
  // Selection logic
  Mesh* selectedMesh = rayCaster.CastRay(meshes);
  const bool pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  
  const bool isHoverObject = selectedMesh != nullptr;
  const bool isSelectObject = isHoverObject && pressed;
  const bool isUnselectObject = !isHoverObject && pressed;
  
  if (isHoverObject && !locked)
    selectedMesh->isSelected = true;
  
  if (isSelectObject)
    locked = true;
  
  if (isUnselectObject)
    locked = false;
  
  if (lastSelectedMesh != nullptr && lastSelectedMesh != selectedMesh && !locked)
    lastSelectedMesh->isSelected = false;
  
  if (!locked)
    lastSelectedMesh = selectedMesh;
  
  // Transformation
  if (locked && lastSelectedMesh != nullptr)
  {
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
      lastSelectedMesh->RotateY(lastSelectedMesh->rotation.y + 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
      lastSelectedMesh->RotateZ(lastSelectedMesh->rotation.z + 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
      lastSelectedMesh->Scale(lastSelectedMesh->scale + 0.1f);
    }
  }
}