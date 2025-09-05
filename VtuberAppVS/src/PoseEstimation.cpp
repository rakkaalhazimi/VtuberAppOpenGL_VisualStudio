#include "PoseEstimation.h"


PoseEstimation::PoseEstimation(const std::wstring& modelPath): 
  env(ORT_LOGGING_LEVEL_WARNING, "test"), 
  session(env, modelPath.c_str(), Ort::SessionOptions{ nullptr })
{
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

}


Ort::Value PoseEstimation::predictYolo11(cv::Mat frame)
{
  // Resize frame to model input size (YOLO11n-pose default is 640x640)
  int input_w = 640, input_h = 640;
  cv::Mat resized;
  cv::resize(frame, resized, cv::Size(input_w, input_h));

  // Convert BGR -> RGB
  cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

  // Convert to float32 and normalize [0,255] -> [0,1]
  resized.convertTo(resized, CV_32F, 1.0 / 255.0);

  // HWC -> CHW (H - Height, W- Width, C - Channel)
  std::vector<float> inputTensorValues;
  inputTensorValues.assign(resized.begin<float>(), resized.end<float>());
  std::vector<float> chw;
  chw.reserve(3 * input_h * input_w);
  for (int c = 0; c < 3; c++) 
  {
    for (int y = 0; y < input_h; y++) 
    {
      for (int x = 0; x < input_w; x++) 
      {
        chw.push_back(resized.at<cv::Vec3f>(y, x)[c]);
      }
    }
  }

  // Get input name
  std::vector<std::string> inputNames = session.GetInputNames();
  std::vector<const char*> inputNamesChar;
  inputNamesChar.reserve(inputNames.size());
  for (std::string &item : inputNames)
  {
    inputNamesChar.push_back(item.c_str());
  }

  //// Create input tensor
  Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  std::array<int64_t, 4> inputShape = { 1, 3, input_h, input_w }; // (b, c, h, w)
  Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
    memoryInfo, chw.data(), chw.size(), inputShape.data(), inputShape.size()
  );

  // Get output name
  std::vector<std::string> outputNames = session.GetOutputNames();
  std::vector<const char*> outputNamesChar;
  outputNamesChar.reserve(outputNames.size());
  for (std::string &item : outputNames)
  {
    outputNamesChar.push_back(item.c_str());
  }

  // Run inference
  std::vector<Ort::Value> output_tensors = session.Run(
    Ort::RunOptions{ nullptr }, inputNamesChar.data(), &input_tensor, 1, outputNamesChar.data(), 1
  );

  // Tensor ONNX can't be copy, we can move it.
  return std::move(output_tensors.front());
}


Ort::Value PoseEstimation::predict(cv::Mat frame)
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
    1 // use only the first output [ 1 2554 12 ]
  );

  std::cout << "Output size: " << output_tensors.size() << std::endl;

  for (size_t i = 0; i < output_tensors.size(); i++)
  {
    Ort::TensorTypeAndShapeInfo tensorInfo = output_tensors[i].GetTensorTypeAndShapeInfo();
    std::vector<int64_t> shape = tensorInfo.GetShape();
    std::cout << "Output " << i << " shape: [ ";
    for (auto dim : shape) std::cout << dim << " ";
    std::cout << "]" << std::endl;
  }

  const float* floatArray = output_tensors[0].GetTensorData<float>();
  std::cout << "[ " << std::endl;
  for (size_t i = 0; i < 10; i++)
  {
    std::cout << floatArray[i] << std::endl;
  }
  std::cout << "] " << std::endl;



  // Tensor ONNX can't be copy, we can move it.
  return std::move(output_tensors.front());
}