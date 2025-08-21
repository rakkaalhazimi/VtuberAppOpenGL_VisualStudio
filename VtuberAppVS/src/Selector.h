#ifndef SELECTOR_CLASS_H
#define SELECTOR_CLASS_H


#include "Mesh.h"
#include "RayCaster.h"
#include "shader.h"


class Selector
{
  
  public:
    Selector();
    void Watch(GLFWwindow* window, RayCaster& rayCaster, std::vector<Mesh*> meshes);
  
  private:
    bool locked = false;
    Mesh* lastSelectedMesh = nullptr;
};

#endif