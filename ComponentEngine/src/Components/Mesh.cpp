#include <ComponentEngine\Components\Mesh.hpp>

#include <ComponentEngine\pugixml.hpp>
#include <EnteeZ\EnteeZ.hpp>

using namespace ComponentEngine;

ComponentEngine::Mesh::Mesh(std::string path) : m_path(path)
{
	m_loaded = false;
}

void ComponentEngine::Mesh::EntityHook(enteez::Entity & entity, pugi::xml_node & component_data)
{

	pugi::xml_node mesh_node = component_data.child("Path");
	if (mesh_node)
	{
		std::string path = mesh_node.attribute("value").as_string();
		Mesh* mesh = entity.AddComponent<Mesh>(path);
		if (!mesh->Loaded())
		{
			std::cout << "Mesh: Unable to find mesh (" << path.c_str() << ")" << std::endl;
			entity.RemoveComponent<Mesh>();
		}
	}
}

std::string ComponentEngine::Mesh::GetPath()
{
	return m_path;
}

bool ComponentEngine::Mesh::Loaded()
{
	return m_loaded;
}
