#pragma once

#include <codecvt>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#ifdef _WIN32
	#include <windows.h>
#endif



namespace Utils
{
	// Encoding
	std::wstring utf8_to_wstring(const std::string& str);
	std::string utf16_to_utf8(const char* c);
	std::string sjis_to_utf8(const char* data);


	// Files and Folder
	template <typename T>
	bool readBinary(std::istream& stream, T& value)
	{
		stream.read(reinterpret_cast<char*>(&value), sizeof(T));
		return stream.good();
	};
	std::ifstream openFile(std::string filepath);


	// Logging
	void printHex(const char* data, size_t size);
};