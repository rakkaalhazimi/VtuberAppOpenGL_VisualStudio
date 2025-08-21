#include "Texture.h"


std::wstring utf8_to_wstring(const std::string& str) {
  int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
  std::wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), wstr.data(), len);
  return wstr;
}


bool endsWith(const std::string& fullString, const std::string& ending) 
{
    if (ending.size() > fullString.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), fullString.rbegin());
}


Texture::Texture(std::string image, const char* texType, GLuint slot)
{
  type = texType;
  
  int widthImg, heightImg, numColCh;
  stbi_set_flip_vertically_on_load(true);
  
  std::wstring imageWidePath = utf8_to_wstring(image);
  std::filesystem::path imagePath = imageWidePath;
  std::ifstream imageFile {imagePath, std::ios::binary};
  unsigned char* bytes;
  
  
  if (!imageFile.is_open())
  {
    std::cerr << "Error opening file: " << imagePath << std::endl;
    std::cerr << "Failed to load image: " << stbi_failure_reason() << std::endl;
    exit(1);
  }
  
  // Temporary solution to open bmp file
  if (endsWith(image, ".bmp"))
  {
    // const char *imageCStr = image.c_str();
    bytes = stbi_load(image.c_str(), &widthImg, &heightImg, &numColCh, 0);
  }
  else
  {
    uint32_t size = std::filesystem::file_size(imagePath);
    // std::cout << "File size: " << size << " bytes" << std::endl;
    std::vector<unsigned char> buffer(size);
    imageFile.read(reinterpret_cast<char*>(buffer.data()), size);
    bytes = stbi_load_from_memory(buffer.data(), size, &widthImg, &heightImg, &numColCh, 0);
    
  }
  
  if (bytes == nullptr)
  {
    std::cerr << "Failed to open image: " << stbi_failure_reason() << std::endl;
    exit(1);
  }
  
  glGenTextures(1, &ID);
  glActiveTexture(GL_TEXTURE0 + slot);
  unit = slot;
  glBindTexture(GL_TEXTURE_2D, ID);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  // std::cout << "Image: " << image << std::endl;
  // std::cout << "width: " << widthImg << std::endl;
  // std::cout << "height: " << heightImg << std::endl;
  // std::cout << "numColCh: " << numColCh << std::endl;
  
  if (numColCh == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	}
	else if (numColCh == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGB, GL_UNSIGNED_BYTE, bytes);
	}
  else if (numColCh == 2)
  {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, widthImg, heightImg, 0, GL_RG, GL_UNSIGNED_BYTE, bytes);
  }
	else if (numColCh == 1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RED, GL_UNSIGNED_BYTE, bytes);
	}
	else 
	{
    std::cerr << "num col ch: " << numColCh << std::endl;
		throw std::invalid_argument("Automatic Texture type recognition failed");
	}
  
  glGenerateMipmap(GL_TEXTURE_2D);
  
  stbi_image_free(bytes);
  imageFile.close();
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
  GLuint texUni = glGetUniformLocation(shader.ID, uniform);
  shader.Activate();
  glUniform1i(texUni, unit);
}

void Texture::Bind()
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
  glDeleteTextures(1, &ID);
}