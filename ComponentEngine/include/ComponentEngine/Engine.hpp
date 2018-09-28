#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include <renderer\IRenderer.hpp>

namespace ComponentEngine
{
	class Engine : public enteez::EnteeZ
	{
	public:
		Engine();
		~Engine();
	private:
		void InitEnteeZ();
		void InitRenderer();
		Renderer::IRenderer* m_renderer = nullptr;
	};
}