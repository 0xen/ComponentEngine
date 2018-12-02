#pragma once

#include <ComponentEngine\Engine.hpp>

namespace ComponentEngine
{
	struct CurrentSceneFocus
	{
		enteez::Entity* entity = nullptr;
		BaseComponentWrapper* component = nullptr;
	};

	class UIMaanger
	{
	public:
		UIMaanger(Engine* engine);
		void Render();
	private:
		void RenderMainMenu();
		void RenderFPSCounter();
		void RenderScene();
		void RenderEntityTreeNode(Entity* entity);
		void RenderEntity(Entity* entity);
		void RenderComponentTreeNode(Entity* entity, BaseComponentWrapper& wrapper);
		void RenderComponent();

		bool ElementClicked();

		Engine* m_engine;
		CurrentSceneFocus m_current_scene_focus;
		unsigned int m_indestructable_component_id = 0;
		static const unsigned int SCENE;
		bool m_open[2];
	};
}
