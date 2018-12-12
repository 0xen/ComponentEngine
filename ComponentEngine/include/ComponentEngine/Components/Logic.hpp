#pragma once

namespace ComponentEngine
{
	class Logic
	{
	public:
		virtual void Update(float frame_time) = 0;
	};
}