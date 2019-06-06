#pragma once

#include <imgui.h>

namespace ComponentEngine
{
	typedef int UIDisplayFlags; // Engine::PlayState
	class Engine;
	class UIBase
	{
	public:
		UIBase(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		void Render();
		virtual void Contents() = 0;
		UIDisplayFlags GetDisplayFlags();
		friend class UIManager;
	private:
		ImGuiWindowFlags m_flags;
		const char* m_title;
		bool m_open;
		UIDisplayFlags m_displayFlags;
	protected:
		Engine * m_engine;
		UIManager* m_manager;
	};
}