#pragma once

#include <string>
#include <map>

#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
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

	class IVertexBuffer;
	class IIndexBuffer;

	struct ModelBuffers
	{
		~ModelBuffers();
		IVertexBuffer* vertexBuffer;
		IIndexBuffer* indexBuffer;
		unsigned int used_instances = 0;
	};

	struct Mesh : public MsgRecive<RenderStatus>
	{
		Mesh(std::string path);
		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);
		std::string GetPath();
		bool Loaded();
		virtual void ReciveMessage(enteez::Entity* sender, const RenderStatus& message);
	private:
		void LoadModel();
		std::string m_path;
		bool m_loaded;
		static std::map<std::string, ModelBuffers> m_mdel_buffer_instances;
	};
}