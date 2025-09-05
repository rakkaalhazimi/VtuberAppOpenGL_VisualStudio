#include "pose/PoseDrawer.h"


PoseDrawer::PoseDrawer()
{
	// Initiate VAO and VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 1 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void PoseDrawer::Draw(Shader &shader, Landmark &landmark, int windowWidth, int windowHeight)
{
	shader.Activate();

	// Projection matrix to show constant 2D object
	glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
	//glm::mat4 projection(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	int width = 256, height = 256;

	//std::cout << "Width: " << landmark.x << std::endl;
	//std::cout << "Height: " << landmark.y << std::endl;
	std::cout << "Confidence: " << landmark.presence << std::endl;
	std::cout << "Visibility: " << landmark.visibility << std::endl;


	float vertices[4] = {landmark.x, landmark.y, 0.0f, 0.0f};
	//float vertices[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Draw vertcies
	glDrawArrays(GL_POINTS, 0, 1);
}