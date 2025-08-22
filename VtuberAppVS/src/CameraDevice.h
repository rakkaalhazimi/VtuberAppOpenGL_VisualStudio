#pragma once

#include<thread>
#include<mutex>

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<opencv2/opencv.hpp>

#include "shader.h"


class CameraDevice
{
public:
	cv::VideoCapture cap;
	cv::Mat frame;
	GLuint tex;
	GLuint VAO, VBO;

	CameraDevice();
	void start(Shader& shader, int windowWidth, int windowHeight, float x, float y);
	cv::Mat getFrame();

private:
	std::thread captureThread;
	std::mutex frameMutex;
	bool running = true;
};