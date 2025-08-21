#include "shader.h"

std::string get_file_content(const char* filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Cannot open file: " << filename << std::endl;
    exit(1);
  }
  
  std::stringstream contentStream;
  contentStream << file.rdbuf();
  std::string content = contentStream.str();
  return content;
}

Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
  std::string vertexCode = get_file_content(vertexFile);
  std::string fragmentCode = get_file_content(fragmentFile);
  
  const char* vertexSource = vertexCode.c_str();
  const char* fragmentSource = fragmentCode.c_str();
  
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  compileErrors(vertexShader);
  
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  compileErrors(fragmentShader);
  
  ID = glCreateProgram();
  glAttachShader(ID, vertexShader);
  glAttachShader(ID, fragmentShader);

  glLinkProgram(ID);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
  glUseProgram(ID);
}

void Shader::Delete()
{
  glDeleteProgram(ID);
}

void Shader::compileErrors(GLuint shader)
{
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      exit(0);
  }
}