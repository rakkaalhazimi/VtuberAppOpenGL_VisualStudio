#ifndef RAY_CASTER_H_CLASS
#define RAY_CASTER_H_CLASS


#include<glad/glad.h>
#include<glfw/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Mesh.h"
#include "shader.h"


class RayCaster
{
  public:
    GLuint VAO;
    GLuint VBO;
    glm::vec3 rayOrigin;
    glm::vec3 rayDirection;
    glm::vec3 vertices[2];
    
    RayCaster();
    void DrawLine();
    void Activate(GLFWwindow* window, Shader& shader, Camera& camera);
    bool Intersect(Shader &shader, Mesh &mesh);
    bool RayIntersectsTriangle(
      const glm::vec3& v0,
      const glm::vec3& v1,
      const glm::vec3& v2,
      float& t, float& u, float& v
    );
    void IntersectRayWithMesh(Mesh* mesh, float& dist);
    Mesh* CastRay(const std::vector<Mesh*> &meshes);
};


#endif
