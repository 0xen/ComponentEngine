#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <renderer\IRenderer.hpp>

#include <ComponentEngine\Components\Transformation.hpp>

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
	private:
		Uint32 GetWindowFlags(RenderingAPI api);
		void InitWindow();
		void UpdateWindow();
		void DeInitWindow();
		void InitEnteeZ();
		void InitRenderer();
		IRenderer* m_renderer = nullptr;
		// Windowing data
		SDL_Window* m_window;
		NativeWindowHandle* m_window_handle;
		bool m_running = true;
		RenderingAPI m_api;
		const char* m_title; 
		int m_width;
		int m_height;
	};
}