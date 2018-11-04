#include <ComponentEngine\Common.hpp>


using namespace ComponentEngine;

std::string ComponentEngine::Common::GetDir(std::string path)
{
	return path.substr(0, path.find_last_of("\\/") + 1);
}
