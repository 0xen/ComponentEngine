#pragma once

#include <string>

#include <ComponentEngine\tiny_obj_loader.h>

namespace enteez
{
	class Entity;
}
namespace pugi
{
	class xml_node;
}

namespace ComponentEngine
{
	struct Mesh
	{
		Mesh(std::string path);
		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);
		std::string GetPath();
		bool Loaded();
	private:
		std::string m_path;
		bool m_loaded;
	};
}