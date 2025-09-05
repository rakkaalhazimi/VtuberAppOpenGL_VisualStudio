/*
Reference:
https://huggingface.co/unity/inference-engine-blaze-pose/tree/main/models
https://github.com/Unity-Technologies/inference-engine-samples/tree/main/BlazeDetectionSample/Pose
*/
#pragma once


#include<cstring>
#include<iostream>

#include "onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"



struct Landmark
{
	float x;
	float y;
	float z;
	float visibility;
	float presence;
};

enum class BlazePoseKeypoint {
  Nose = 0,
  LeftEyeInner,
  LeftEye,
  LeftEyeOuter,
  RightEyeInner,
  RightEye,
  RightEyeOuter,
  LeftEar,
  RightEar,
  MouthLeft,
  MouthRight,
  LeftShoulder,
  RightShoulder,
  LeftElbow,
  RightElbow,
  LeftWrist,
  RightWrist,
  LeftPinky,
  RightPinky,
  LeftIndex,
  RightIndex,
  LeftThumb,
  RightThumb,
  LeftHip,
  RightHip,
  LeftKnee,
  RightKnee,
  LeftAnkle,
  RightAnkle,
  LeftHeel,
  RightHeel,
  LeftFootIndex,
  RightFootIndex
};



class BlazePose
{
public:
	Ort::Env env;
	Ort::Session session;
	Ort::AllocatorWithDefaultOptions allocator;

	BlazePose(const std::wstring& modelPath);
	std::vector<Landmark> predict(cv::Mat frame);
};
