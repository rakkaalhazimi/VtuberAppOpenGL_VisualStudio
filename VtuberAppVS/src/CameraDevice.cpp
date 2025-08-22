#include "CameraDevice.h"


CameraDevice::CameraDevice()
{
	// Start device camera
	cap = cv::VideoCapture(0);
	int w = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
	int h = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

	// Create Texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Initiate VAO and VBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Thread for camera capture
	captureThread = std::thread([this]() 
	{
		cv::Mat newFrame;
		while (running) 
		{
			cap >> newFrame;
			if (!newFrame.empty()) {
				std::lock_guard<std::mutex> lock(frameMutex);
				newFrame.copyTo(frame);   // update shared frame
			}
		}
	});

}


cv::Mat CameraDevice::getFrame()
{
	std::lock_guard<std::mutex> lock(frameMutex);
	if (frame.empty())
	{
		return cv::Mat();
	}
	return frame.clone();   // safe copy for external use
}


void CameraDevice::start(Shader& shader, int windowWidth, int windowHeight, float x, float y)
{
	shader.Activate();

	// Projection matrix to show constant 2D object
	glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Copy latest frame (non-blocking)
	cv::Mat localFrame;
	{
		std::lock_guard<std::mutex> lock(frameMutex);
		if (!frame.empty())
			frame.copyTo(localFrame);
	}

	// Update texture with new frame
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	if (!localFrame.empty())
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.cols, frame.rows, GL_BGR, GL_UNSIGNED_BYTE, frame.data);
		GLuint texUni = glGetUniformLocation(shader.ID, "camera");
		glUniform1i(texUni, 0);
	}


	int width = 300, height = 300;

	// Update vertices (v2 vertex, v2 texCoord)
	float vertices[6][4] = 
	{
			{ x,					y,          0.0f, 0.0f }, // top-left
			{ x + width,  y,          1.0f, 0.0f }, // top-right
			{ x + width,  y + height, 1.0f, 1.0f }, // bottom-right

			{ x,					y,						0.0f, 0.0f }, // top-left
			{ x + width,  y + height,   1.0f, 1.0f }, // bottom-right
			{ x,					y + height,   0.0f, 1.0f }, // bottom-left
	};
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Draw vertcies
	glDrawArrays(GL_TRIANGLES, 0, 6);
}