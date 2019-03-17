#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <ComponentEngine\Components\MsgRecive.hpp>


namespace ComponentEngine
{
	template<typename T>
	void Send(enteez::Entity* target, enteez::Entity* sender, T data, bool sendToChildren = false);
}

#include <ComponentEngine\Components\Transformation.hpp>

namespace ComponentEngine
{
	template<typename T>
	void Send(enteez::Entity* target, enteez::Entity* sender, T data, bool sendToChildren)
	{
		target->ForEach<MsgRecive<T>>([&data,&sender](enteez::Entity* entity, MsgRecive<T>& recive)
			{
				recive.ReciveMessage(sender, data);
			});
		if (sendToChildren && target->HasComponent<Transformation>())
		{
			std::vector<Transformation*> children = target->GetComponent<Transformation>().GetChildren();
			for (int i = 0; i < children.size(); i++)
			{
				Send(children[i]->GetEntity(), sender, data, sendToChildren);
			}
		}
	}
}