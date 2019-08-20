#pragma once

#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\IO.hpp>
#include <ComponentEngine\Components\TransferBuffers.hpp>

#include <glm\glm.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace Renderer
{
	namespace Vulkan
	{
		//class VulkanBufferPool;
	}
}
namespace ComponentEngine
{
	struct LightData
	{
		glm::vec3 position;
		float intensity;
		glm::vec3 color;
		float alive;
	};
	class Light : public Logic, public IO, public UI
	{
	public:
		Light(enteez::Entity* entity);
		~Light();

		virtual void Update(float frame_time);
		virtual void EditorUpdate(float frame_time);
		virtual void Display();

		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	private:
		glm::vec3 m_offset;
		float m_intensity;
		glm::vec3 m_color;

		unsigned int m_light_allocation;
		//VulkanBufferPool* m_light_pool;
		enteez::Entity* m_entity;
	};
}