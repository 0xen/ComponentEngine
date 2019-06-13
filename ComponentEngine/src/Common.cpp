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

bool ComponentEngine::Common::Contains(std::string target, std::string search)
{
	return target.find(search) != std::string::npos;
}

void ComponentEngine::Common::Read(std::ifstream & in, void * ptr, unsigned long long size)
{
	in.read((char *)ptr, size);
}

std::string ComponentEngine::Common::ReadString(std::ifstream & in)
{

	unsigned int stringsize;
	Read(in, &stringsize, sizeof(unsigned int));

	char* data = new char[stringsize];
	Read(in, data, sizeof(char) * stringsize);
	std::string str;
	str.assign(data);
	return str;
}

void ComponentEngine::Common::Write(std::ofstream & out, void * ptr, unsigned long long size)
{
	out.write((char *)ptr, size);
}

void ComponentEngine::Common::Write(std::ofstream & out, std::string & str)
{
	unsigned int size = str.size() + 1; // +1 for \0 at the end of the string to define where it ends
	// Strings size
	Write(out, &size, sizeof(unsigned int));
	// String data
	Write(out, (void*)str.data(), sizeof(char) * size);
}