#include "pose/BlazePose.h"


BlazePose::BlazePose(const std::wstring& modelPath):
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

  //std::cout << "Output size: " << output_tensors.size() << std::endl;

  /*for (size_t i = 0; i < output_tensors.size(); i++)
  {
    Ort::TensorTypeAndShapeInfo tensorInfo = output_tensors[i].GetTensorTypeAndShapeInfo();
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