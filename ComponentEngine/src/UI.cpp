#include <ComponentEngine\UI.hpp>
#include <imgui.h>

const unsigned int ComponentEngine::UI::SCENE = 0;

ComponentEngine::UI::UI(Engine* engine) : m_engine(engine)
{
}

void ComponentEngine::UI::Render()
{
	ImGui::NewFrame();

	ImGui::ShowTestWindow();


	RenderMainMenu();
	RenderFPSCounter();
	RenderScene();


	ImGui::Render();
}

void ComponentEngine::UI::RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				m_engine->Stop();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void ComponentEngine::UI::RenderFPSCounter()
{
	ImVec2 window_pos = ImVec2(10.0f, 25.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
	bool open = true;
	if (ImGui::Begin("", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("FPS:%i  TPS:%f", (int)(1.0f / m_engine->GetFrameTime()), 0.0f);
		//ImGui::Separator();
	}
	ImGui::End();
}

void ComponentEngine::UI::RenderScene()
{
	if (ImGui::Begin("Scene", &m_open[SCENE], ImGuiWindowFlags_NoCollapse))
	{
		EntityManager& em = m_engine->GetEntityManager();
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		{
			ImGui::BeginChild("Child1", ImVec2((ImGui::GetWindowContentRegionWidth() * 0.48f), 320), false, ImGuiWindowFlags_HorizontalScrollbar);
			{
				int i = 0;
				for (auto entity : em.GetEntitys())
				{
					ImGui::PushID(i);
					RenderEntityTreeNode(entity);
					ImGui::PopID();
					i++;
				}
				ImGui::EndChild();
			}
		}

		ImGui::NextColumn();

		{
			ImGui::BeginChild("Child2", ImVec2((ImGui::GetWindowContentRegionWidth() * 0.48f), 320), false, ImGuiWindowFlags_HorizontalScrollbar);
			{

				if (m_current_scene_focus.entity != nullptr)
				{

					// Title
					{
						ImGui::Columns(2);
						ImGui::Separator();
						ImGui::Text("Entity");
						ImGui::NextColumn();
						ImGui::Text(m_current_scene_focus.entity->GetName().c_str());
						ImGui::Columns(1);
						ImGui::Separator();
					}
					RenderEntity(m_current_scene_focus.entity);
				}

				if (m_current_scene_focus.component != nullptr)
				{
					// Add padding between Entity and component
					ImGui::NewLine();
					ImGui::NewLine();
					// Title
					{
						ImGui::Columns(2);
						ImGui::Separator();
						ImGui::Text("Component");
						ImGui::NextColumn();
						ImGui::Text(m_current_scene_focus.component->GetName().c_str());
						ImGui::Columns(1);
						ImGui::Separator();
					}
					RenderComponent(m_current_scene_focus.entity, *m_current_scene_focus.component);
				}
				ImGui::EndChild();
			}
		}
		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();

	}
	ImGui::End();
}

void ComponentEngine::UI::RenderEntityTreeNode(Entity * entity)
{
	bool node_open = ImGui::TreeNode("GameObject", "%s", entity->GetName().c_str());


	if (ElementClicked())
	{
		m_current_scene_focus.entity = entity;
		m_current_scene_focus.component = nullptr;
	}

	if (node_open)
	{
		entity->ForEach([entity, this](BaseComponentWrapper& wrapper)
		{
			RenderComponentTreeNode(entity, wrapper);
		});
		ImGui::TreePop();
	}
}

void ComponentEngine::UI::RenderEntity(Entity * entity)
{
}

void ComponentEngine::UI::RenderComponentTreeNode(Entity* entity, BaseComponentWrapper & wrapper)
{
	bool click = ImGui::TreeNodeEx("Component", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "%s", wrapper.GetName().c_str());

	if (ElementClicked())
	{
		m_current_scene_focus.entity = entity;
		m_current_scene_focus.component = &wrapper;
	}

}

void ComponentEngine::UI::RenderComponent(Entity * entity, BaseComponentWrapper & wrapper)
{
}

bool ComponentEngine::UI::ElementClicked()
{
	return ImGui::IsItemHovered() && ImGui::IsMouseClicked(0);
}
