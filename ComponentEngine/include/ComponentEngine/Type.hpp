#pragma once

#include <string>

namespace ComponentEngine
{
	class Type
	{
	public:
		template<typename T>
		bool InstanceOf();
	protected:
		const std::type_info* m_type_info;
	};
	template<typename T>
	inline bool Type::InstanceOf()
	{
		return *m_type_info == typeid(T);
	}
}