#pragma once

#include<cstring>
#include<iostream>

#include "onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"


class PoseEstimation
{
	public:
		Ort::Env env;
		Ort::Session session;
		Ort::AllocatorWithDefaultOptions allocator;

		PoseEstimation(const std::wstring& modelPath);
		Ort::Value predictYolo11(cv::Mat frame);
		Ort::Value predict(cv::Mat frame);
};
