#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <renderer\IRenderer.hpp>

#include <ComponentEngine\Components\Transformation.hpp>
#include <ComponentEngine\Components\Camera.hpp>
#include <ComponentEngine\DefaultMeshVertex.hpp>

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
		bool Running();
		void Update();
		void Render();
		IGraphicsPipeline* GetDefaultGraphicsPipeline();
		IRenderer* GetRenderer();
		float GetFrameTime();
	private:
		Uint32 GetWindowFlags(RenderingAPI api);
		void InitWindow();
		void UpdateWindow();
		void DeInitWindow();
		void InitEnteeZ();
		void DeInitEnteeZ();
		void InitRenderer();
		void DeInitRenderer();

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
		Camera m_camera;
		IUniformBuffer* m_camera_buffer;
		IDescriptorPool* m_camera_pool;

		// FPS
		float m_delta_time = 0;
		float m_now_delta_time = 0;
		float m_frame_time = 0;
		unsigned int m_delta_fps = 0;
		unsigned int m_fps = 0;


	};
}