#include "PMXDrawer.h"



PMXDrawer::PMXDrawer(std::vector<VertexModel> &vertices, std::vector<GLuint> &indices)
{
  // OpenGL Array Buffer
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexModel), vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

  // Position (vec3)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)0);
  glEnableVertexAttribArray(0);

  // Normal (vec3)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // UV (vec2)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexModel), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // Bone Indices (ivec4)
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexModel), (void*)(8 * sizeof(float)));
  glEnableVertexAttribArray(3);

  // Bone Weights (vec4)
  glVertexAttribPointer(4, 4, GL_INT, GL_FALSE, sizeof(VertexModel), (void*)(8 * sizeof(float) + 4 * sizeof(int)));
  glEnableVertexAttribArray(4);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void PMXDrawer::Draw(
  Shader& shader,
  std::vector<VertexModel> &vertices,
  std::vector<glm::mat4> &boneMatrices,
  std::vector<PMXMaterial> &materials,
  std::vector<Texture> &textures
)
{
  shader.Activate();
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(VertexModel), vertices.data());

  int indexOffset = 0;

  for (PMXMaterial& material : materials)
  {
    int indexCount = material.faceCount;
    int textureIndex = material.textureIndex;
    int environmentIndex = material.environmentIndex;

    textures[textureIndex].texUnit(shader, "myTexture", material.textureIndex);
    textures[textureIndex].Bind();

    if (material.environmentMode > 0 && material.environmentIndex != -1)
    {
      textures[environmentIndex].texUnit(shader, "envTexture", environmentIndex);
      textures[environmentIndex].Bind();
    }

    glUniform1i(
      glGetUniformLocation(shader.ID, "envMode"),
      material.environmentMode
    );

    glUniform4f(
      glGetUniformLocation(shader.ID, "diffuseColor"),
      material.diffuseColor.x, material.diffuseColor.y, material.diffuseColor.z, material.diffuseColor.w
    );

    glUniform3f(
      glGetUniformLocation(shader.ID, "ambientColor"),
      material.ambientColor.x, material.ambientColor.y, material.ambientColor.z
    );

    glUniform3f(
      glGetUniformLocation(shader.ID, "specularColor"),
      material.specularColor.x, material.specularColor.y, material.specularColor.z
    );

    glUniform1f(
      glGetUniformLocation(shader.ID, "shininess"),
      material.specularity
    );


    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(indexOffset * sizeof(GLuint)));

    indexOffset += indexCount;
  }
}
