#pragma once

#include <EnteeZ\EnteeZ.hpp>

namespace ComponentEngine
{
	class MsgSend
	{
	public:
		MsgSend() {}
		MsgSend(enteez::Entity* entity) : m_entity(entity) {}
		template<typename T>
		void Send(T data)
		{
			m_entity->ForEach<MsgRecive<T>>([data](enteez::Entity* entity, MsgRecive<T>& recive)
			{
				recive.ReciveMessage(entity, data);
			});
		}
	private:
		enteez::Entity* m_entity;
	};

}