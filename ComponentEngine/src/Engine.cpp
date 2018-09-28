#include <ComponentEngine\Engine.hpp>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

ComponentEngine::Engine::Engine()
{
	m_title = "Component Engine";
	m_width = 1080;
	m_height = 720;
	m_api = RenderingAPI::VulkanAPI;
	InitWindow();
	InitEnteeZ();
	InitRenderer();
}

ComponentEngine::Engine::~Engine()
{
	delete m_renderer;
	DeInitWindow();
}

bool ComponentEngine::Engine::Running()
{
	return m_running;
}

void ComponentEngine::Engine::Update()
{
	UpdateWindow();
}

Uint32 ComponentEngine::Engine::GetWindowFlags(RenderingAPI api)
{
	switch (api)
	{
	case VulkanAPI:
		return SDL_WINDOW_VULKAN;
		break;
	}
	return 0;
}

void ComponentEngine::Engine::InitWindow()
{
	m_window = SDL_CreateWindow(
		m_title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		m_width, m_height,
		GetWindowFlags(m_api) | SDL_WINDOW_RESIZABLE
	);
	SDL_ShowWindow(m_window);

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	assert(SDL_GetWindowWMInfo(m_window, &info) && "Error, unable to get window info");

	m_window_handle = new NativeWindowHandle(info.info.win.window, m_width, m_height);
	m_window_handle->clear_color = { 0.2f,0.2f,0.2f,1.0f };
}

void ComponentEngine::Engine::UpdateWindow()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			m_running = false;
			break;
		}
	}
}

void ComponentEngine::Engine::DeInitWindow()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			m_running = false;
			break;
		}
	}
}

void ComponentEngine::Engine::InitEnteeZ()
{

}

void ComponentEngine::Engine::InitRenderer()
{
}
