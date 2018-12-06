#pragma once

#include<ComponentEngine\Type.hpp>

namespace ComponentEngine
{
	template<class T>
	class TemplateType : public Type
	{
	public:
		TemplateType()
		{
			m_type_info = &typeid(T);
		}
	};
}