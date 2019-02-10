#pragma once

#include <glm/glm.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\TransferBuffers.hpp>

#include <vector>

namespace enteez
{
	class Entity;
}

namespace pugi
{
	class xml_node;
}
namespace Renderer
{
	class IUniformBuffer;
}

using namespace Renderer;

namespace ComponentEngine
{

	class Transformation;

	class Camera : public Logic, public UI, public TransferBuffers
	{

	public:
		Camera();
		Camera(enteez::Entity* entity);
		~Camera();

		virtual void Update(float frame_time);
		virtual void SetBufferData();
		virtual void BufferTransfer();
		virtual void Display();

		static void EntityHookDefault(enteez::Entity& entity);
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);

		void SetMainCamera();
		IUniformBuffer* GetCameraBuffer();
		void UpdateProjection();

		Transformation* GetTransformation();
	private:

		enteez::Entity* m_entity = nullptr;
		IUniformBuffer* m_camera_buffer;

		static std::vector<Camera*> m_global_cameras;

		float m_near_clip;
		float m_far_clip;
		float m_fov;

		// Camera buffer data
		struct {

			glm::mat4 view;
			glm::mat4 projection;
		}m_camera_data;
	};

}