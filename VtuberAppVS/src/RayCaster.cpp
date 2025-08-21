#include "RayCaster.h"


RayCaster::RayCaster()
{
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)(sizeof(glm::vec3)));
  // glEnableVertexAttribArray(2);
  std::cout << "Initiate ray caster" << std::endl;

  glBindVertexArray(0);
}

void RayCaster::Activate(GLFWwindow* window, Shader& shader, Camera& camera)
{
  shader.Activate();
  
  // Get mouse coordinate
  double xpos, ypos;
  int winWidth, winHeight;
  glfwGetWindowSize(window, &winWidth, &winHeight);
  glfwGetCursorPos(window, &xpos, &ypos);
  
  float x = (2.0f * xpos) / winWidth - 1.0f;
  float y = 1.0f - (2.0f * ypos) / winHeight;
  glm::vec2 ndc = glm::vec2(x, y);
  
  glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
  glm::vec4 rayEye = glm::inverse(camera.projection) * rayClip;
  rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f); // Direction in eye space
  
  // The line is pointing to our direction, that is why it seems like following the mouse.
  glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(camera.view) * rayEye));
  rayDirection = rayWorld;
  
  // It's a point in world space, near the camera (on the near clipping plane). 
  // If your camera is far from the origin or zoomed out, these values may look small.
  glm::vec4 mousePoint = glm::inverse(camera.projection * camera.view) * glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);
  mousePoint /= mousePoint.w;
  rayOrigin = glm::vec3(mousePoint.x, mousePoint.y, mousePoint.z - 0.01f);
  
  // Update vertices
  vertices[0] = rayOrigin;
  vertices[1] = rayOrigin + rayWorld * 100.0f;
}

void RayCaster::DrawLine()
{
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBindVertexArray(VAO);
  glDrawArrays(GL_LINES, 0, 2);  
}


bool RayCaster::RayIntersectsTriangle(
  const glm::vec3& v0,
  const glm::vec3& v1,
  const glm::vec3& v2,
  float& t, float& u, float& v)
{
  const float EPSILON = 1e-8;
  glm::vec3 edge1 = v1 - v0;
  glm::vec3 edge2 = v2 - v0;
  
  glm::vec3 h = glm::cross(rayDirection, edge2);
  float a = glm::dot(edge1, h);
  
  // Parallel to the triangle if a is closer to 0
  if (fabs(a) < EPSILON)
  {
    return false;
  }
  
  float f = 1.0 / a;
  glm::vec3 s = rayOrigin - v0;
  u = f * glm::dot(s, h);
  
  // Point u is outside the triangle
  if (u < 0.0 || u > 1.0)
  {
    return false;
  }
  
  glm::vec3 q = glm::cross(s, edge1);
  v = f * glm::dot(rayDirection, q);
  
  // Point v is outside the triangle
  if (v < 0.0 || u + v > 1.0)
  {
    return false;
  }
  
  t = f * glm::dot(edge2, q);
  
  // Ray intersect triangle if t is more than Epsilon
  return t > EPSILON;
}


bool RayCaster::Intersect(Shader &shader, Mesh &mesh)
{
  float closestT = FLT_MAX;
  bool hit = false;
  glm::vec3 hitPoint;
  
  for (size_t i = 0; i < mesh.indices.size(); i += 3) 
  {
    glm::vec3 v0 = mesh.positionsTransformed[mesh.indices[i]];
    glm::vec3 v1 = mesh.positionsTransformed[mesh.indices[i + 1]];
    glm::vec3 v2 = mesh.positionsTransformed[mesh.indices[i + 2]];

    float t, u, v;
    if (RayIntersectsTriangle(v0, v1, v2, t, u, v)) 
    {
      if (t < closestT) 
      {
          closestT = t;
          hit = true;
          // hitPoint = rayOriginLocal + t * rayDirLocal;
      }
    }
  }
  return hit;
  // End for loop
}

void RayCaster::IntersectRayWithMesh(Mesh* mesh, float& dist)
{
  float closestT = FLT_MAX;
  // std::vector<glm::vec3> positions = mesh->getTransformedPosition();
  for (size_t i = 0; i < mesh->indices.size(); i += 3) 
  {
    glm::vec3 v0 = mesh->positionsTransformed[mesh->indices[i]];
    glm::vec3 v1 = mesh->positionsTransformed[mesh->indices[i + 1]];
    glm::vec3 v2 = mesh->positionsTransformed[mesh->indices[i + 2]];
    float t, u, v;
    
    if (!RayIntersectsTriangle(v0, v1, v2, t, u, v)) continue;
    
    if (t < closestT)
    {
      closestT = t;
    }
  }
  dist = closestT;
}


Mesh* RayCaster::CastRay(const std::vector<Mesh*> &meshes)
{
  Mesh* closestMesh = nullptr;
  float closestDistance = FLT_MAX;
  
  for (Mesh* mesh : meshes)
  {
    float dist;
    IntersectRayWithMesh(mesh, dist);
    
    if (dist < closestDistance)
    {
      closestDistance = dist;
      closestMesh = mesh;
    }
  }
  
  return closestMesh;
}