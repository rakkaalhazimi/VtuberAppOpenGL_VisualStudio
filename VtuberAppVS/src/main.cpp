#include<filesystem>
#include<iomanip>
#include<iostream>
#include<fstream>
#include<unordered_map>
#include<set>
#include<sstream>

#ifdef _WIN32
  #include <windows.h>
#endif

#include<glad/glad.h>
#include<glfw/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<imgui/imgui.h>
#include<imgui/imgui_impl_glfw.h>
#include<imgui/imgui_impl_opengl3.h>

#include<opencv2/opencv.hpp>

#include "Camera.h"
#include "CameraDevice.h"
#include "commands/CommandManager.h"
#include "commands/RotateBoneCommand.h"
#include "gui/PMXEditorGUI.h"
#include "Mesh.h"
#include "PMXFile.h"
#include "PMXModel.h"
#include "RayCaster.h"
#include "Selector.h"
#include "Texture.h"
#include "TextRenderer.h"



const unsigned int width = 1280;
const unsigned int height = 720;

// position | normal | color | texture
Vertex vertices[] = 
{
  // Upper left
  Vertex{ glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },
  // Upper right
  Vertex{ glm::vec3(0.5f, 0.5, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(2.0f, 1.0f) },
  // Lower left
  Vertex{ glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },
  // Lower right
  Vertex{ glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.5f), glm::vec2(2.0f, 0.0f) },
};


GLuint indices[] = {
  0, 1, 2,
  1, 2, 3,
};



int main(int argc, char * argv[]) {


  std::cout << "Current working dir: " << std::filesystem::current_path() << std::endl;
  
  if (argc < 2)
  {
      std::cout << "Missing filename argument" << std::endl;
      return 1;
  }
  std::string modelFilepath = argv[1];
  std::cout << "Model filepath: " << modelFilepath << std::endl;
  
  // Enable windows to print kanji
  #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
  #endif
  
  
  // Initialize GLFW
  glfwInit();
  
  // Tell GLFW what version of OpenGL we are using
  // In this case, we are using OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // Tell GLFW we are using the CORE profile
  // So that means we only have the modern functions
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  // Create a GLFWwindow object of 800 by 800 pixels, naming it YoutubeOpenGL
  GLFWwindow *window = glfwCreateWindow(width, height, "Vtuber Application", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
  }
  
  // Introduce the windows into the current context
  glfwMakeContextCurrent(window);
  
  // Vsync ON
  glfwSwapInterval(1);
  
  // Load GLAD so it configures OpenGL
  gladLoadGL();
  
  // Specify the viewport of OpenGL in the Window
  // In this case the viewport goes from x=0, y=0, to x=800, y=800
  glViewport(0, 0, width, height);
  
  
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("./assets/fonts/mgenplus-1mn-bold.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();
  
  
  Shader shader("assets/shaders/default.vert", "assets/shaders/default.frag");
  Shader textShader("assets/shaders/text.vert", "assets/shaders/text.frag");
  Shader rayShader("assets/shaders/default.vert", "assets/shaders/ray.frag");
  Shader pmxShader("assets/shaders/pmx.vert", "assets/shaders/default.frag");
  Shader camDeviceShader("assets/shaders/camera-device.vert", "assets/shaders/camera-device.frag");
  
  
  // Texture
  Texture myTexture("assets/images/brs.png", "textureType", 0);
  // myTexture.texUnit(shader, "myTexture", 0);
  
  // Mesh
  std::vector<Vertex> vert(vertices, vertices + sizeof(vertices) / sizeof(Vertex));
  std::vector<GLuint> ind(indices, indices + sizeof(indices) / sizeof(GLuint));
  std::vector<Texture> tex = {myTexture};
  
  Mesh mesh(vert, ind, tex);
  Mesh mesh2(vert, ind, tex);
  mesh2.translation.x = 1.5f;
  std::vector<Mesh*> meshes = {&mesh, &mesh2};
  
  
  // Raycaster
  RayCaster rayCaster;
  
  
  // UV Scrolling
  const float targetFPS = 60.0f;
  const float scrollSpeed = 0.2f;  // How fast the texture scrolls (units per second)
  const float targetFrameTime = 1.0f / targetFPS;  // Time per frame (1 second / target FPS)

  float lastTime = glfwGetTime();
  float elapsedTime = 0.0f;
  float uOffset = 0.0f;
  
  // Camera
  Camera camera(width, height, glm::vec3(0.0f, 20.0f, -15.0f));
  
  glEnable(GL_DEPTH_TEST);
  // Specify the color of the background
  glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
  // Clean the back buffer and assign the new color to it
  glClear(GL_COLOR_BUFFER_BIT);
  // Swap the back buffer with the front buffer
  glfwSwapBuffers(window);
  
  
  // Freetype
  TextRenderer textRender("assets/fonts/ARIAL.TTF");
  bool locked = false;
  
  glm::mat4 model = glm::mat4(1.0f);
  
  
  // Selector
  Selector selector;
  
  
  // PMXFile
  PMXFile pmxFile(modelFilepath.c_str());
  
  std::vector<Texture> pmxTextures = {};
  for (int i = 0; i < pmxFile.textures.size(); i++)
  {
    pmxTextures.push_back(
      Texture{pmxFile.textures[i].c_str(), "myTexture", (GLuint)i}
    );
  }
  
  std::vector<Vertex> pmxVertices;
  for (PMXVertex item: pmxFile.vertices)
  {
    pmxVertices.push_back(
      Vertex 
      {
        item.position, 
        item.normal, 
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec2(item.uv.x, 1.0f - item.uv.y), // Flips vertical for PMX File
      }
    );
  }
  
  std::vector<GLuint> pmxIndices;
  for (uint16_t item: pmxFile.indices)
  {
    pmxIndices.push_back(item);
  }
  Mesh feixiaoMesh(pmxVertices, pmxIndices, pmxTextures);
  
  int faceAllCount = 0;
  for (PMXMaterial item: pmxFile.materials)
  {
    faceAllCount += item.faceCount;
  }
  
  // PMX Model
  PMXModel feixiaoModel(pmxFile);
  
  float modelXRotation = 0.0f;
  float modelYRotation = 0.0f;
  
  
  // Moving limb test
  // 55 = Left Arm Index
  // 60 = Left Elbow Index
  // 65 = Left Wrist Index
  // 16 = Head Index
  int base = 65;
  int target = 60;
  glm::vec3 vectorBase = glm::normalize(feixiaoModel.bones[base].restPosition - feixiaoModel.bones[target].restPosition);
  std::cout 
    << "Vector base: " 
    << vectorBase.x
    << " "
    << vectorBase.y
    << " "
    << vectorBase.z << std::endl;
    
  glm::vec3 vectorTarget = glm::vec3(0.2f, 0.8f, 0.4f);
  
  glm::vec3 axis = glm::cross(vectorBase, vectorTarget); // vector that perpendicular with a and b.
  float angle = std::acos(glm::dot(vectorBase, vectorTarget));
  
  glm::quat rotation = angleAxis(angle, normalize(axis));
  
  // Convert to matrix and then to Euler angles
  glm::mat4 rotationMatrix = glm::toMat4(rotation);
  glm::vec3 eulerAngles = glm::eulerAngles(rotation);  // or extract manually

  // Apply Euler angles to bone
  feixiaoModel.bones[target].rotation.x = eulerAngles.x;  // depends on your rotation order
  feixiaoModel.bones[target].rotation.y = eulerAngles.y;
  feixiaoModel.bones[target].rotation.z = eulerAngles.z;
  
  // Move whole model
  feixiaoModel.bones[3].position.x = 2.0f;
  
  
  // Morph test
  // feixiaoModel.UpdateMorph();
  float morphWeight = 0.0f;
  glm::vec3 boneRotation(0.0f);
  RotateBoneCommand command(feixiaoModel, 16, boneRotation);
  CommandManager commandManager;
  
  // GUI test
  PMXEditorGUI pmxEditorGUI(feixiaoModel, commandManager);

  // Camera Device
  CameraDevice cameraDevice;
  
  
  // Main while loop
  while (!glfwWindowShouldClose(window))
  {
    // Take care of all GLFW events
    glfwPollEvents();
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    pmxEditorGUI.draw();
    
    ImGui::ShowDemoWindow(); // Show demo window! :)
    
    if (morphWeight > 0.0f)
    {
      feixiaoModel.UpdateMorph(morphWeight);
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // UV Scrolling
    elapsedTime += deltaTime;
    if (elapsedTime >= targetFrameTime)
    {
      shader.Activate();
      // uOffset += 0.01f; // temporarily disable uvOffset for pmx mesh
      modelXRotation += 1.0f;
      modelYRotation += 0.01f;
      GLuint uOffsetLoc = glGetUniformLocation(shader.ID, "uOffset");
      glUniform1f(uOffsetLoc, uOffset);
      
      elapsedTime = 0.0f;
    }
    
    // Camera
    camera.Inputs(window);
    camera.updateMatrix(45.0f, 0.1f, 100.0f, shader, "camMatrix");
    
    shader.Activate();
    selector.Watch(window, rayCaster, meshes);
    mesh.Draw(shader);
    mesh2.Draw(shader);
    
    pmxShader.Activate();
    
    camera.updateMatrix(45.0f, 0.1f, 100.0f, pmxShader, "camMatrix");
    feixiaoModel.Update();
    feixiaoModel.Draw(pmxShader);
    
    // Ray Casting
    rayShader.Activate();
    camera.updateMatrix(45.0f, 0.1f, 100.0f, rayShader, "camMatrix");
    rayCaster.Activate(window, rayShader, camera);
    rayCaster.DrawLine();
    const bool hit = rayCaster.Intersect(shader, mesh);
    const bool pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool unlocked = glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS;

    // Device Camera
    cameraDevice.start(camDeviceShader, width, height, 0.0f, 0.0f);
    
    // Get mouse coordinate
	  double xpos, ypos;
    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);
	  glfwGetCursorPos(window, &xpos, &ypos);
    float x = (2.0f * xpos) / winWidth - 1.0f;
    float y = 1.0f - (2.0f * ypos) / winHeight;
    glm::vec2 ndc = glm::vec2(x, y);
    
    std::stringstream mouseLog;
    mouseLog 
      << "X: " << std::setprecision(3) << ndc.x
      << " "
      << "Y: " << std::setprecision(3) << ndc.y
      << " "
      // << "Z: " << std::setprecision(3) << rayCaster.rayDirection.z
      << " "
      << "Inter: " << hit;
      // << "width: " << winWidth << " height: " << winHeight;
    // textRender.type(textShader, mouseLog.str(), 400.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));
    
    // Rendering Imgui
    // (After clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
  }
  
  shader.Delete();
  textShader.Delete();
  rayShader.Delete();
  mesh.Delete();
  myTexture.Delete();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  
  return 0;
}