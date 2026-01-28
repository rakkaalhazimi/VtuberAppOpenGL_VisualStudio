#include "Utils.h"



std::wstring Utils::utf8_to_wstring(const std::string& str)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), nullptr, 0);
  std::wstring wstr(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), wstr.data(), len);
  return wstr;
}


std::string Utils::utf16_to_utf8(const char* c)
{
  // buffer contains UTF-16LE data
  int32_t length = 20;
  //std::cout << "Strlen: " << length << std::endl;
  auto utf16 = reinterpret_cast<const wchar_t*>(c);
  int utf16Length = length / sizeof(wchar_t);

  int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16Length, nullptr, 0, nullptr, nullptr);

  // Convert UTF-16LE -> UTF-8
  std::string result(sizeNeeded, 0);
  WideCharToMultiByte(CP_UTF8, 0, utf16, utf16Length, &result[0], sizeNeeded, nullptr, nullptr);

  return result;
}


std::string Utils::sjis_to_utf8(const char* data)
{
  /*size_t len = 0;
  while (len < maxSize && data[len] != '\0')
    ++len;*/
  int sjisLen = static_cast<int>(strlen(data));

  // Shift-JIS -> UTF-16
  int wideLen = MultiByteToWideChar(932, 0, data, sjisLen, nullptr, 0);
  std::wstring wide(wideLen, 0);
  MultiByteToWideChar(932, 0, data, sjisLen, wide.data(), wideLen);

  // UTF-16 -> UTF-8
  int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, nullptr, 0, nullptr, nullptr);
  std::string utf8(utf8Len, 0);
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), wideLen, utf8.data(), utf8Len, nullptr, nullptr);

  return utf8;
}



std::ifstream Utils::openFile(std::string filepath)
{
  std::wstring filepathWide = utf8_to_wstring(filepath);
  std::filesystem::path path = filepathWide;
  std::ifstream file{ path, std::ios::binary };
  if (!file.is_open()) {
    std::cerr << "Can't open file: " << filepath << std::endl;
  }
  return file;
}


void Utils::printHex(const char* data, size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    std::cout
      << std::hex << std::setw(2) << std::setfill('0')
      << (static_cast<unsigned int>(static_cast<unsigned char>(data[i])))
      << ' ';
  }
  std::cout << std::dec << '\n';
}


void Utils::printVector(glm::vec3& vector, const char* name)
{
  std::cout << name << " : " << vector.x << " " << vector.y << " " << vector.z << std::endl;
}