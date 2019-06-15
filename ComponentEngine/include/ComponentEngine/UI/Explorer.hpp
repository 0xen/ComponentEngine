#pragma once

#include <ComponentEngine\UI\UIBase.hpp>
#include <ComponentEngine\UI\UIManager.hpp>

#include <functional>

namespace ComponentEngine
{
	class Explorer : public UIBase
	{
	public:
		Explorer(const char* title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open = true);

		virtual void Contents();

		static void LoadFolder(Folder & folder);
		static void RendererFolder(Folder & folder);
		static void RendererFolder(Folder & folder, std::function<void(const char* path)> doubleClickCallBack);
	private:
		Folder m_sceneFolder;
	};
}