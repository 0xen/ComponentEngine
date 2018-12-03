#pragma once

namespace ComponentEngine
{
	struct MouseClickEvent
	{
		short button;
		float state;
	};
	struct MouseMoveEvent
	{
		float x;
		float y;
	};
}