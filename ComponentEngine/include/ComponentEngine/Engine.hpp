#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <renderer\IRenderer.hpp>

#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Camera.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>
#include <ComponentEngine\ThreadHandler.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <SDL.h>
#include <SDL_syswm.h>

using namespace enteez;
using namespace Renderer;

namespace ComponentEngine
{
	class Engine : public EnteeZ
	{
	public:
		Engine();
		~Engine();
		void Start(void(*logic_function)());
		bool Running();
		void Update();
		void Rebuild();
		void RenderFrame();
		IGraphicsPipeline* GetDefaultGraphicsPipeline();
		IRenderer* GetRenderer();
		Entity* GetCameraEntity();
		Transformation* GetCameraTransformation();
		IDescriptorPool* GetCameraPool();
		IDescriptorSet* GetCameraDescriptorSet();;
		float GetFrameTime();
		float GetThreadTime();
		float GetFPS();

		ordered_lock& GetLogicMutex();
		ordered_lock& GetRendererMutex();
	private:
		Uint32 GetWindowFlags(RenderingAPI api);
		void InitWindow();
		void UpdateWindow();
		void DeInitWindow();
		void InitEnteeZ();
		void DeInitEnteeZ();
		void InitRenderer();
		void DeInitRenderer();

		void UpdateCameraProjection();

		// Windowing data
		SDL_Window* m_window;
		NativeWindowHandle* m_window_handle;
		RenderingAPI m_api;
		const char* m_title; 
		int m_width;
		int m_height;

		// Rendering Data
		IRenderer* m_renderer = nullptr;

		// Default pipeline data
		IGraphicsPipeline* m_default_pipeline = nullptr;
		Entity* m_camera_entity;
		Camera m_camera_component;
		IUniformBuffer* m_camera_buffer;
		IDescriptorPool* m_camera_pool;
		IDescriptorSet* m_camera_descriptor_set;

		// FPS
		Uint64 m_delta_time = 0;
		Uint64 m_now_delta_time = 0;
		float m_frame_time = 0;
		float m_fps_update = 0.0f;
		unsigned int m_delta_fps = 0;
		float m_fps = 0;


		ThreadHandler* m_logic_thread;
		ordered_lock m_renderer_thread;

		std::map<std::thread::id, Uint64> m_thread_time;

		static const unsigned int IS_RUNNING_LOCK;
		static const unsigned int THREAD_TIME_LOCK;

		// Store the various locks that will be needed
		std::mutex m_locks[2];
	};
}