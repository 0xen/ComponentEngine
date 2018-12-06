#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>

namespace ComponentEngine
{
	template<typename T>
	void Send(enteez::Entity* current_entity, T data)
	{
		current_entity->ForEach<MsgRecive<T>>([&data](enteez::Entity* entity, MsgRecive<T>& recive)
		{
			recive.ReciveMessage(entity, data);
		});
	}
	
	/*class MsgSend
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
	};*/

}