#pragma once

namespace ComponentEngine
{
	class Logic
	{
	public:
		virtual void EditorUpdate(float frame_time) {};
		virtual void Update(float frame_time) = 0;
	};
}