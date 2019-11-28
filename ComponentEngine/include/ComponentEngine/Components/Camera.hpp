#pragma once

#include <glm/glm.hpp>
#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <ComponentEngine\Components\TransferBuffers.hpp>
#include <ComponentEngine\Components\IO.hpp>

#include <vector>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace Renderer
{
	namespace Vulkan
	{
		class VulkanUniformBuffer;
	}
}

using namespace Renderer;
using namespace Renderer::Vulkan;


namespace ComponentEngine
{

	class Transformation;

	class Camera : public Logic, public UI, public TransferBuffers, public IO
	{

	public:
		Camera();
		Camera(enteez::Entity* entity);
		~Camera();

		virtual void Update(float frame_time);
		virtual void SetBufferData();
		virtual void BufferTransfer();
		virtual void Display();

		void DisplayRaytraceConfig();

		virtual void Load(std::ifstream& in);
		virtual void Save(std::ofstream& out);
		virtual unsigned int PayloadSize();
		virtual bool DynamiclySized();

		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);

		void SetMainCamera();
		VulkanUniformBuffer* GetCameraBuffer();
		void UpdateProjection();

		Transformation* GetTransformation();
	private:

		void SendDataToGPU();

		enteez::Entity* m_entity = nullptr;
		VulkanUniformBuffer* m_camera_buffer;

		static std::vector<Camera*> m_global_cameras;

		float m_near_clip;
		float m_far_clip;
		float m_fov;

		// Camera buffer data
		struct {

			glm::mat4 view;
			glm::mat4 proj;

			// #VKRay
			glm::mat4 viewInverse;
			glm::mat4 projInverse;

			unsigned int recursionCount;
			// Camera Settings
			unsigned int samplesPerFrame;
			float aperture;
			float focusDistance;
			float movmentTollarance;
			unsigned int totalSamples;
		}m_camera_data;
	};

}