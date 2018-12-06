#pragma once

#include <EnteeZ\EnteeZ.hpp>

namespace ComponentEngine
{
	template<typename T>
	class MsgRecive
	{
	public:
		virtual void ReciveMessage(enteez::Entity* sender, T& message) = 0;
	private:

	};
}