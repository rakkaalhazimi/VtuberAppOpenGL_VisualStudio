/*
Reference:
https://huggingface.co/unity/inference-engine-blaze-pose/tree/main/models
https://github.com/Unity-Technologies/inference-engine-samples/tree/main/BlazeDetectionSample/Pose
https://medium.com/axinc-ai/blazepose-a-3d-pose-estimation-model-d8689d06b7c4
*/
#pragma once


#include<algorithm>
#include<cmath>
#include<cstring>
#include<iostream>
#include<memory>

#include "onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"



class Region
{
  public:
    float pdScore;
    std::vector<float> pdBox;
    std::vector<std::vector<float>> pdKps;

    float rectW;
    float rectH;
    float rectXCenter;
    float rectYCenter;

    float rectXCenterA;
    float rectYCenterA;
    
    float rectWA;
    float rectHA;

    std::vector<cv::Point2f> rectPoints;

    float rotation;

};


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
	std::shared_ptr<Ort::Session> pdSession;
  std::shared_ptr<Ort::Session> lmSession;
	Ort::AllocatorWithDefaultOptions allocator;
  std::vector<std::vector<float>> anchors; // predefined cx, cy, scale_x and scale_y

  int pdScale = 224;
  double pi = std::atan(1.0) * 4;

	BlazePose(const std::wstring& modelPath);
  std::vector<Ort::Value> pdInference(cv::Mat frame); // pose detection
  
  Region decodeBboxes(Ort::Value &scores, Ort::Value &bboxes);
  Region detectionToRect(Region &region);
  std::vector<cv::Point2f> rotatedRectToPoints(
    float cx, float cy, float w, float h, float rotation, float wi, float hi
  );
  void rectTransformation(Region &region, float w, float h);
  
  cv::Mat warpRectImg(std::vector<cv::Point2f> &rectPoints, cv::Mat &img, float w, float h);
  void lmInference(); // landmark

	std::vector<Landmark> predict(cv::Mat frame);
};
