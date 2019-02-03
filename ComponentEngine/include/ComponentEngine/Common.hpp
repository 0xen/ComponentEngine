#pragma once

#include <string>

namespace ComponentEngine
{
	namespace Common 
	{
		std::string GetDir(std::string path);
		float RandomNumber(float Min, float Max);
		void Replace(std::string& str, std::string old, std::string newStr);
	}
}