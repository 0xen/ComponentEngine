#include <MouseDrag.hpp>

#include <ComponentEngine\Engine.hpp>

#include <EnteeZ\EnteeZ.hpp>

#include <imgui.h>

#include <sstream>

ComponentEngine::MouseDrag::MouseDrag(enteez::Entity * entity)
{

	m_entity = entity;
}

void ComponentEngine::MouseDrag::Update(float frame_time)
{
	Engine* engine = Engine::Singlton();
	/*if(engine->MouseKeyDown(0))
	{
		engine->GrabMouse(true);
		std::stringstream ss;
		ss << engine->GetLastMouseMovment().x << " " << engine->GetLastMouseMovment().y;
		engine->Log(ss.str());
	}
	else
	{
		engine->GrabMouse(false);
	}*/
}

void ComponentEngine::MouseDrag::EditorUpdate(float frame_time)
{

}

void ComponentEngine::MouseDrag::Display()
{

}

void ComponentEngine::MouseDrag::Load(std::ifstream & in)
{
	ReadBinary(in, reinterpret_cast<char*>(this) + offsetof(MouseDrag, m_rotateSpeed), PayloadSize());
}

void ComponentEngine::MouseDrag::Save(std::ofstream & out)
{
	WriteBinary(out, reinterpret_cast<char*>(this) + offsetof(MouseDrag, m_rotateSpeed), PayloadSize());
}

unsigned int ComponentEngine::MouseDrag::PayloadSize()
{
	return SizeOfOffsetRange(MouseDrag, m_rotateSpeed, m_rotateSpeed);
}

bool ComponentEngine::MouseDrag::DynamiclySized()
{
	return false;
}

enteez::BaseComponentWrapper * ComponentEngine::MouseDrag::EntityHookDefault(enteez::Entity & entity)
{
	enteez::ComponentWrapper<MouseDrag>* wrapper = entity.AddComponent<MouseDrag>(&entity);
	wrapper->SetName("Mouse Drag");
	return wrapper;
}
