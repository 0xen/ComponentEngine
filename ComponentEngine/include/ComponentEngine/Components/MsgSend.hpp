#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>


namespace ComponentEngine
{
	template<typename T>
	void Send(enteez::Entity* current_entity, T data, bool sendToChildren = false);
}

#include <ComponentEngine\Components\Transformation.hpp>

namespace ComponentEngine
{
	template<typename T>
	void Send(enteez::Entity * current_entity, T data, bool sendToChildren)
	{
		current_entity->ForEach<MsgRecive<T>>([&data](enteez::Entity* entity, MsgRecive<T>& recive)
			{
				recive.ReciveMessage(entity, data);
			});
		if (sendToChildren && current_entity->HasComponent<Transformation>())
		{
			std::vector<Transformation*> children = current_entity->GetComponent<Transformation>().GetChildren();
			for (int i = 0; i < children.size(); i++)
			{
				Send(children[i]->GetEntity(), data, sendToChildren);
			}
		}
	}
}