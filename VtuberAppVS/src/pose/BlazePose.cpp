#include "pose/BlazePose.h"


BlazePose::BlazePose(const std::wstring& modelPath):
  env(ORT_LOGGING_LEVEL_WARNING, "test"),
  session(env, modelPath.c_str(), Ort::SessionOptions{ nullptr })
{

  Ort::Env pdEnv(ORT_LOGGING_LEVEL_WARNING, "test");
  std::wstring pdModelPath = L"assets/dnn/pose_detection.onnx";
  pdSession = std::make_shared<Ort::Session>(pdEnv, pdModelPath.c_str(), Ort::SessionOptions{ nullptr });

  Ort::Env lmEnv(ORT_LOGGING_LEVEL_WARNING, "test");
  lmSession = std::make_shared<Ort::Session>(env, modelPath.c_str(), Ort::SessionOptions{ nullptr });

  // Number of input nodes
  size_t num_input_nodes = session.GetInputCount();
  size_t num_output_nodes = session.GetOutputCount();

  std::cout << "Model loaded successfully!\n";
  std::cout << "Inputs: " << num_input_nodes << ", Outputs: " << num_output_nodes << "\n";

  for (size_t i = 0; i < num_input_nodes; i++)
  {
    Ort::AllocatedStringPtr inputNamePtr = session.GetOutputNameAllocated(i, allocator);
    char* inputName = inputNamePtr.get();
    std::cout << "Input " << i << " name: " << inputName << std::endl;

    // Get input type info
    Ort::TypeInfo type_info = session.GetInputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    ONNXTensorElementDataType type = tensor_info.GetElementType();

    // Get output shape
    std::vector<int64_t> input_shape = tensor_info.GetShape();
    std::cout << "Input " << i << " shape: [ ";
    for (auto dim : input_shape) std::cout << dim << " ";
    std::cout << "]" << std::endl;
  }

  for (size_t i = 0; i < num_output_nodes; i++)
  {
    Ort::AllocatedStringPtr outputNamePtr = session.GetOutputNameAllocated(i, allocator);
    char* outputName = outputNamePtr.get();
    std::cout << "Output " << i << " name: " << outputName << std::endl;

    // Get output type info
    Ort::TypeInfo type_info = session.GetOutputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
    ONNXTensorElementDataType type = tensor_info.GetElementType();

    // Get output shape
    std::vector<int64_t> output_shape = tensor_info.GetShape();
    std::cout << "Output " << i << " shape: [ ";
    for (auto dim : output_shape) std::cout << dim << " ";
    std::cout << "]" << std::endl;
  }

  // Read anchors file
  std::ifstream file("assets/dnn/anchors.csv");
  if (!file.is_open()) 
  {
    std::cerr << "Failed to open csv file.\n";
  }

  std::string line;
  while (std::getline(file, line))
  {
    std::vector<float> row;
    std::stringstream ss(line);
    std::string value;

    while (std::getline(ss, value, ','))
    {
      // remove leading/trailing spaces
      value.erase(0, value.find_first_not_of(" \t"));
      value.erase(value.find_last_not_of(" \t") + 1);

      row.push_back(std::stof(value)); // convert to float
    }

    if (row.size() == 4) { anchors.push_back(std::move(row)); }
    else { std::cerr << "Warning: unexpected number of columns (" << row.size() << ")\n"; }
  }

  file.close();



}


std::vector<Ort::Value> BlazePose::pdInference(cv::Mat frame)
{
  // Resize frame to pose detector input size
  int inputW = 224, inputH = 224;
  cv::Mat resized;
  cv::resize(frame, resized, cv::Size(inputW, inputH));

  // Convert BGR -> RGB
  cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

  // Convert to float32 and normalize [0,255] -> [0,1]
  resized.convertTo(resized, CV_32F, 1.0 / 255.0);

  // Copy cv2 image to vector
  std::vector<float> inputTensorValues(resized.total() * resized.channels());
  std::memcpy(inputTensorValues.data(), resized.data, inputTensorValues.size() * sizeof(float));

  // Get Input Names
  std::vector<std::string> inputNames = pdSession->GetInputNames();
  std::vector<const char*> inputNamesChar;
  inputNamesChar.reserve(inputNames.size());
  for (std::string& item : inputNames)
  {
    inputNamesChar.push_back(item.c_str());
  }

  // Get Output Names
  std::vector<std::string> outputNames = pdSession->GetOutputNames();
  std::vector<const char*> outputNamesChar;
  outputNamesChar.reserve(outputNames.size());
  for (std::string& item : outputNames)
  {
    outputNamesChar.push_back(item.c_str());
  }

  // Create input tensor
  //Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::array<int64_t, 4> inputShape = { 1, inputH, inputW, 3 }; // (b, h, w, c)
  Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
    memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size()
  );

  // Run inference
  std::vector<Ort::Value> outputTensors = pdSession->Run(
    Ort::RunOptions{ nullptr },
    inputNamesChar.data(),
    &inputTensor,
    inputNames.size(),
    outputNamesChar.data(),
    outputNames.size()
  );

  /*for (size_t i = 0; i < outputTensors.size(); i++)
  {
    Ort::TensorTypeAndShapeInfo tensorInfo = outputTensors[i].GetTensorTypeAndShapeInfo();
    std::vector<int64_t> shape = tensorInfo.GetShape();
    std::cout << "Output " << i << " shape: [ ";
    for (auto dim : shape) std::cout << dim << " ";
    std::cout << "]" << std::endl;
  }*/

  return outputTensors;
}


Region BlazePose::decodeBboxes(Ort::Value &scores, Ort::Value &bboxes)
{
  // scores shape: (1, 2554, 1)
  // bboxes shape: (1, 2554, 12)
  
  // Apply sigmoid to scores
  float* scoresData = scores.GetTensorMutableData<float>();
  size_t scoresSize = scores.GetTensorTypeAndShapeInfo().GetElementCount();
  for (size_t i = 0; i < scoresSize; i++)
  {
    scoresData[i] = 1.0f / (1.0f + std::exp(-scoresData[i]));
  }

  // Find the detection scores, bboxes and anchors
  auto maxIt = std::max_element(scoresData, scoresData + scoresSize);
  float maxValue = *maxIt;
  auto maxIndex = std::distance(scoresData, maxIt);

  float *bboxesData = bboxes.GetTensorMutableData<float>();
  auto shape = bboxes.GetTensorTypeAndShapeInfo().GetShape(); // (1, 2554, 12)

  float detScores = scoresData[maxIndex];
  std::vector<float> detBboxes(
    bboxesData + maxIndex * shape[2],
    bboxesData + maxIndex * shape[2] + shape[2]
  );
  std::vector<float> detAnchors = anchors[maxIndex]; // cx, cy, w, h

  /*std::cout << "max index: " << maxIndex << std::endl;
  std::cout << "det scores: " << detScores << std::endl;
  std::cout << "det bboxes: [" << std::endl;
  for (size_t i = 0; i < detBboxes.size(); i++)
  {
    std::cout << " " << detBboxes[i] << std::endl;
  }
  std::cout << "]" << std::endl;
  std::cout << "Anchors: " << std::endl;
  for (auto item : detAnchors)
  {
    std::cout << item << std::endl;
  }*/

  // Make the position of det_bboxes relative to anchor
  // cx = cx * anchor.w / wi + anchor.x_center
  // cy = cy * anchor.h / hi + anchor.y_center
  // etc.
  for (size_t i = 0; i < detBboxes.size(); i += 2)
  {
    detBboxes[i] = detBboxes[i] * detAnchors[2] / pdScale + detAnchors[0];
    detBboxes[i + 1] = detBboxes[i + 1] * detAnchors[3] / pdScale + detAnchors[1];
  }
  // The 2nd and 3rd index are width and height, not a keypoint.
  // We need to revert the addition of anchor.x_center and anchor.y_center.
  detBboxes[2] -= detAnchors[0];
  detBboxes[3] -= detAnchors[1];

  // box = [cx - w * 0.5, cy - h * 0.5, w, h]
  detBboxes[0] -= detBboxes[2] * 0.5f;
  detBboxes[1] -= detBboxes[3] * 0.5f;

  /*std::cout << "det bboxes relative: [" << std::endl;
  for (size_t i = 0; i < detBboxes.size(); i++)
  {
    std::cout << " " << detBboxes[i] << std::endl;
  }
  std::cout << "]" << std::endl;*/

  Region region;

  region.pdScore = detScores;
  region.pdBox = std::vector<float>(detBboxes.begin(), detBboxes.begin() + 4); // First 4 values (cx, cy, w, h)
  for (size_t i = 0; i < 4; i++)
  {
    // Fetch the rest of 8 values (x1, y1, x2, y2, ..., x4, y4).
    // We will have 4 keypoints (kp1, kp2, kp3, kp4)
    std::vector<float> kp = { detBboxes[4 + i*2], detBboxes[4 + i*2 + 1] };
    region.pdKps.push_back(kp);
  }

  return region;
}


Region BlazePose::detectionToRect(Region &region)
{
  double targetAngle = pi * 0.5; // 90-degree = pi/2
  
  float xCenter = region.pdKps[0][0];
  float yCenter = region.pdKps[0][1];
  float xScale = region.pdKps[1][0];
  float yScale = region.pdKps[1][1];

  float boxSize = std::sqrt(
    std::pow((xScale - xCenter), 2) + 
    std::pow((yScale - yCenter), 2)
  ) * 2;

  region.rectW = boxSize;
  region.rectH = boxSize;
  region.rectXCenter = xCenter;
  region.rectYCenter = yCenter;

  float rotation = targetAngle - std::atan2(-(yScale - yCenter), xScale - xCenter);
  region.rotation = rotation;

  /*std::cout << "Box Size: " << boxSize << std::endl;
  std::cout << "Rect X Center: " << xCenter << std::endl;
  std::cout << "Rect Y Center: " << yCenter << std::endl;
  std::cout << "Rotation: " << rotation << std::endl;*/

  return region;
}


std::vector<cv::Point2f> BlazePose::rotatedRectToPoints(
  float cx, float cy, float w, float h, float rotation, float wi, float hi
)
{
  float b = std::cos(rotation) * 0.5f;
  float a = std::sin(rotation) * 0.5f;

  float p0x = cx - a * h - b * w;
  float p0y = cy + b * h - a * w;
  float p1x = cx + a * h - b * w;
  float p1y = cy - b * h - a * w;
  float p2x = (int)(2 * cx - p0x);
  float p2y = (int)(2 * cy - p0y);
  float p3x = (int)(2 * cx - p1x);
  float p3y = (int)(2 * cy - p1y);
  
  p0x = (int)p0x;
  p0y = (int)p0y;
  p1x = (int)p1x;
  p1y = (int)p1y;
  
  return { {p0x, p0y}, {p1x, p1y}, {p2x, p2y}, {p3x, p3y} };
}


void BlazePose::rectTransformation(Region& region, float w, float h)
{
  float scaleX = 1.25;
  float scaleY = 1.25;
  float shiftX = 0;
  float shiftY = 0;

  float width = region.rectW;
  float height = region.rectH;
  float rotation = region.rotation;
  if (rotation == 0)
  {
    region.rectXCenterA = (region.rectXCenter + width * shiftX) * w;
    region.rectYCenterA = (region.rectYCenter + height * shiftY) * h;
  }
  else
  {
    float xShift = (w * width * shiftX * std::cos(rotation) - h * height * shiftY * std::sin(rotation));
    float yShift = (w * width * shiftX * std::sin(rotation) + h * height * shiftY * std::cos(rotation));
    region.rectXCenterA = region.rectXCenter * w + xShift;
    region.rectYCenterA = region.rectYCenter * h + yShift;
  }

  // square_long: true
  float longSide = std::max(width * w, height * h);
  region.rectWA = longSide * scaleX;
  region.rectHA = longSide * scaleY;

  /*std::cout << "rectXCenterA: " << region.rectXCenterA << std::endl;
  std::cout << "rectYCenterA: " << region.rectYCenterA << std::endl;
  std::cout << "rectWA: " << region.rectWA << std::endl;
  std::cout << "rectHA: " << region.rectHA << std::endl;*/

  region.rectPoints = rotatedRectToPoints(
    region.rectXCenterA,
    region.rectYCenterA,
    region.rectWA,
    region.rectHA,
    region.rotation,
    w,
    h
  );

}


cv::Mat BlazePose::warpRectImg(std::vector<cv::Point2f> &rectPoints, cv::Mat &img, float w, float h)
{
  std::vector<cv::Point2f> src(rectPoints.begin() + 1, rectPoints.end());
  std::vector<cv::Point2f> dst = 
  {
    cv::Point2f(0.f, 0.f),
    cv::Point2f((float)w, 0.f),
    cv::Point2f((float)w, (float)h)
  };

  cv::Mat mat = getAffineTransform(src, dst);
  cv::Mat warped;
  warpAffine(img, warped, mat, cv::Size(w, h));
  return warped;
}


std::vector<Landmark> BlazePose::predict(cv::Mat frame)
{
  // Resize frame to model input size (blazpose default is 256x256)
  int input_w = 256, input_h = 256;
  cv::Mat resized;
  cv::resize(frame, resized, cv::Size(input_w, input_h));

  // Convert BGR -> RGB
  cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

  // Convert to float32 and normalize [0,255] -> [0,1]
  resized.convertTo(resized, CV_32F, 1.0 / 255.0);

  //std::cout << "Channels: " << resized.channels() << std::endl;

  // Flatten the tensor into a vector
  std::vector<float> inputTensorValues(resized.total() * resized.channels());
  std::memcpy(inputTensorValues.data(), resized.data, inputTensorValues.size() * sizeof(float));

  //std::cout << "Input size: " << inputTensorValues.size() << std::endl;

  // Get input name
  std::vector<std::string> inputNames = session.GetInputNames();
  std::vector<const char*> inputNamesChar;
  inputNamesChar.reserve(inputNames.size());
  for (std::string& item : inputNames)
  {
    inputNamesChar.push_back(item.c_str());
  }

  // Create input tensor
  //Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::array<int64_t, 4> inputShape = { 1, input_h, input_w, 3 }; // (b, h, w, c)
  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
    memoryInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size()
  );

  // Get output name
  std::vector<std::string> outputNames = session.GetOutputNames();
  std::vector<const char*> outputNamesChar;
  outputNamesChar.reserve(outputNames.size());
  for (std::string& item : outputNames)
  {
    outputNamesChar.push_back(item.c_str());
  }

  // Run inference
  std::vector<Ort::Value> output_tensors = session.Run(
    Ort::RunOptions{ nullptr },
    inputNamesChar.data(),
    &input_tensor,
    inputNames.size(),
    outputNamesChar.data(),
    1 // use only the first output [ 1 195 ]
  );

  //std::cout << "Output size: " << outputTensors.size() << std::endl;

  /*for (size_t i = 0; i < outputTensors.size(); i++)
  {
    Ort::TensorTypeAndShapeInfo tensorInfo = outputTensors[i].GetTensorTypeAndShapeInfo();
    std::vector<int64_t> shape = tensorInfo.GetShape();
    std::cout << "Output " << i << " shape: [ ";
    for (auto dim : shape) std::cout << dim << " ";
    std::cout << "]" << std::endl;
  }*/

  Ort::TensorTypeAndShapeInfo tensorInfo = output_tensors[0].GetTensorTypeAndShapeInfo();
  size_t floatCount = tensorInfo.GetElementCount();
  const float* floatArray = output_tensors[0].GetTensorData<float>();
  
  /*std::cout << "[ " << std::endl;
  for (size_t i = 0; i < 10; i++)
  {
    std::cout << floatArray[i] << std::endl;
  }
  std::cout << "] " << std::endl;*/

  std::vector<Landmark> landmarks;
  for (size_t i = 0; i < floatCount; i += 5)
  {
    landmarks.push_back(
      {
        floatArray[i + 0],
        floatArray[i + 1],
        floatArray[i + 2],
        floatArray[i + 3],
        floatArray[i + 4],
      }
     );
  }
  //std::cout << "Landmarks size: " << landmarks.size() << std::endl;


  // Tensor ONNX can't be copy, we can move it.
  return landmarks;
}