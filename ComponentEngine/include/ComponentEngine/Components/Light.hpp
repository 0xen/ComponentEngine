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

namespace ComponentEngine
{
	// Define the memory structure for the light GPU payload
	struct LightData
	{
		glm::vec3 position; // Position in 3D space (Point light only)
		float intensity; // How bright the light is
		glm::vec3 color; // Lights color
		int alive; // Is the light enabled or not
		int lightType; // Define the type of light. 0: Point light, 1: Directional Light
		glm::vec3 dir; // Directional light direction
	};
	class Light : public Logic, public IO, public UI
	{
	public:
		Light(enteez::Entity* entity);
		~Light();
		// Called during logic updates
		virtual void Update(float frame_time);
		// Called during updates when we are not in the play state
		virtual void EditorUpdate(float frame_time);
		// Called when we are in a ImGui UI draw state and the components info needs to be rendered
		virtual void Display();
		// Load the component from a file
		virtual void Load(std::ifstream& in);
		// Save the component to a file
		virtual void Save(std::ofstream& out);
		// How much data should we save to a file
		virtual unsigned int PayloadSize();
		// Is the size we save to a file dynamic?
		virtual bool DynamiclySized();
		// Define a static constructor for the component
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);
	private:
		glm::vec3 m_offset;
		float m_intensity;
		glm::vec3 m_color;
		glm::vec3 m_dir;
		int m_type;
		const char* m_light_types[2] = { "Point Light", "Directional Light" };

		unsigned int m_light_allocation;
		//VulkanBufferPool* m_light_pool;
		enteez::Entity* m_entity;
	};
}