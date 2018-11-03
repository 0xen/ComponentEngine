#pragma once

namespace ComponentEngine
{
	struct RenderStatus
	{
		RenderStatus(bool should_renderer) : should_renderer(should_renderer) {}
		bool should_renderer;
	};
}