#pragma once

#include <string>
#include <fstream>

namespace ComponentEngine
{
	namespace Common 
	{
		std::string GetDir(std::string path);
		float RandomNumber(float Min, float Max);
		void Replace(std::string& str, std::string old, std::string newStr);
		bool Contains(std::string target, std::string search);

		void Read(std::ifstream& in, void* ptr, unsigned long long size);
		std::string ReadString(std::ifstream& in);

		unsigned int StreamStringSize(std::string& str);


		void Write(std::ofstream& out, void* ptr, unsigned long long size);
		void Write(std::ofstream& out, const std::string& str);

	}
}