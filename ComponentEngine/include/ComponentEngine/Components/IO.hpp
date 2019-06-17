#pragma once

#include <fstream>

namespace ComponentEngine
{


	// This will give you the size diffrence between m and n, n size inclusize
	#define SizeOfOffsetRange(s,m,n) ((::size_t) (offsetof(s, n) - offsetof(s, m)) + sizeof(((s*)0)->n)/*Calculate the size of the last element*/ )

	#define WriteBinary(out,ptr,size) out.write((char *)ptr, size)
	#define ReadBinary(v,ptr,size) in.read((char *)ptr, size)

	class IO
	{
	public:
		virtual void Load(std::ifstream& in) = 0;
		virtual void Save(std::ofstream& out) = 0;
		virtual unsigned int PayloadSize() = 0;
		virtual bool DynamiclySized() = 0;
	};
}
