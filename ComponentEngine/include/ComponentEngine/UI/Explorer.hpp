#pragma once

#include <ComponentEngine\UI\UIBase.hpp>
#include <ComponentEngine\UI\UIManager.hpp>

namespace ComponentEngine
{
	class Explorer : public UIBase
	{
	public:
		Explorer(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();

	private:
		void LoadFolder(Folder & folder);
		void RendererFolder(Folder & folder);
		Folder m_sceneFolder;
	};
}