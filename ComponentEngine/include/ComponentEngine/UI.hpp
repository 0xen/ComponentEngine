#pragma once

#include <ComponentEngine\Engine.hpp>

namespace ComponentEngine
{

	struct CurrentSceneFocus
	{
		enteez::Entity* entity = nullptr;
		BaseComponentWrapper* component = nullptr;
	};

	class UI
	{
	public:
		UI(Engine* engine);
		void Render();
	private:
		void RenderMainMenu();
		void RenderFPSCounter();
		void RenderScene();
		void RenderEntityTreeNode(Entity* entity);
		void RenderEntity(Entity* entity);
		void RenderComponentTreeNode(Entity* entity, BaseComponentWrapper& wrapper);
		void RenderComponent(Entity* entity, BaseComponentWrapper& wrapper);

		bool ElementClicked();

		Engine* m_engine;
		CurrentSceneFocus m_current_scene_focus;

		static const unsigned int SCENE;
		bool m_open[2];
	};
}
