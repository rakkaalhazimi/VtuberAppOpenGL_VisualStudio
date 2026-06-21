#pragma once

#include <vector>

#include <glad/glad.h>

#include "PMXModel.h"


class PMXDrawer
{
	public:
		GLuint VAO, VBO, EBO;

		PMXDrawer(std::vector<VertexModel> &vertices, std::vector<GLuint> &indices);
    void Draw(
      Shader& shader,
      std::vector<VertexModel> &vertices,
      std::vector<glm::mat4> &boneMatrices,
      std::vector<PMXMaterial> &materials,
      std::vector<Texture> &textures
     );
};
