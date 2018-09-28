#include <ComponentEngine\Engine.hpp>

using namespace ComponentEngine;
using namespace enteez;
using namespace Renderer;

ComponentEngine::Engine::Engine()
{
	InitEnteeZ();
	InitRenderer();
}

ComponentEngine::Engine::~Engine()
{
	delete m_renderer;
}

void ComponentEngine::Engine::InitEnteeZ()
{

}

void ComponentEngine::Engine::InitRenderer()
{
}
