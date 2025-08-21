#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector <Texture>& textures)
{
  Mesh::vertices = vertices;
  Mesh::indices = indices;
  Mesh::textures = textures;
  
  for (Vertex item: vertices)
  {
    positions.push_back(item.position);
    positionsTransformed.push_back(item.position);
  }
  
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  
  glBindVertexArray(VAO);
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(9 * sizeof(float)));
  glEnableVertexAttribArray(3);
  
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  // Initial Centroid
  centroid = glm::vec3(0.0f);
  for (int i = 0; i < positions.size(); i++)
  {
    centroid += positions[i];
  }
  centroid /= static_cast<float>(positions.size());
}

void Mesh::Draw(Shader& shader)
{
  shader.Activate();
  glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
  
  // Textures
  unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;
  
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;
		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
		}
		textures[i].texUnit(shader, "myTexture", i);
		textures[i].Bind();
	}
  
  // Selection Color
  if (isSelected)
  {
    glUniform3f(glGetUniformLocation(shader.ID, "selectionColor"), 0.0f, 1.0f, 0.0f);
  }
  else
  {
    glUniform3f(glGetUniformLocation(shader.ID, "selectionColor"), 1.0f, 1.0f, 1.0f);
  }
  
  // Transformation
  model = glm::mat4(1.0f);
  
  // Translation
  model = glm::translate(model, translation);
  
  // Rotation
  model = glm::translate(model, centroid);
  model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
  model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
  model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
  model = glm::translate(model, -centroid);
  
  // Scale
  model = glm::scale(model, glm::vec3(scale));
  
  TransformPosition(model);
  glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
  
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::DrawPMX(Shader &shader, std::vector<PMXMaterial> &materials)
{
  shader.Activate();
  glBindVertexArray(VAO);
  
  int indexOffset = 0;
  
  for (int i = 0; i < materials.size(); i++)
  {
    
    int indexCount = materials[i].faceCount;
    int textureIndex = materials[i].textureIndex;
    
    textures[textureIndex].texUnit(shader, "myTexture", textureIndex);
    textures[textureIndex].Bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)(indexOffset * sizeof(GLuint)));
    
    indexOffset += indexCount;
  }
}


void Mesh::RotateZ(float degree)
{
  rotation.z = degree;
}

void Mesh::RotateY(float degree)
{
  rotation.y = degree;
}

void Mesh::Scale(float scale)
{
  Mesh::scale = scale;
}

void Mesh::TransformPosition(glm::mat4& model)
{
  for (int i = 0; i < positionsTransformed.size(); i++)
  {
    glm::vec4 transformed = model * glm::vec4(positions[i], 1.0f);
    positionsTransformed[i] = glm::vec3(transformed);
  }
}

std::vector<glm::vec3> Mesh::getTransformedPosition()
{
  std::vector<glm::vec3> newPositions;
  for (glm::vec3 item: positions)
  {
    glm::vec3 transformed = glm::vec3(model * glm::vec4(item, 1.0f));
    newPositions.push_back(transformed);
  }
  return newPositions;
};

void Mesh::Delete()
{
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}