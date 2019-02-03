#include <ComponentEngine\Common.hpp>


using namespace ComponentEngine;

std::string ComponentEngine::Common::GetDir(std::string path)
{
	return path.substr(0, path.find_last_of("\\/") + 1);
}
float ComponentEngine::Common::RandomNumber(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

void ComponentEngine::Common::Replace(std::string & str, std::string old, std::string newStr)
{
	std::string::size_type pos = 0u;
	while ((pos = str.find(old, pos)) != std::string::npos) {
		str.replace(pos, old.length(), newStr);
		pos += newStr.length();
	}
}
