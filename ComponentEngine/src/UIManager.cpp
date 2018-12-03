#include <ComponentEngine\UIManager.hpp>
#include <ComponentEngine\Components\Indestructable.hpp>
#include <ComponentEngine\Components\UI.hpp>

#include <imgui.h>

const unsigned int ComponentEngine::UIMaanger::SCENE = 0;
const unsigned int ComponentEngine::UIMaanger::ADD_COMPONENT = 1;

ComponentEngine::UIMaanger::UIMaanger(Engine* engine) : m_engine(engine)
{
	m_indestructable_component_id = engine->GetEntityManager().GetBaseIndex<Indestructable>();
}

void ComponentEngine::UIMaanger::Render()
{
	ImGui::NewFrame();
	//ImGui::ShowTestWindow();


	RenderMainMenu();
	RenderFPSCounter();
	if (m_open[SCENE]) RenderScene();
	if (m_open[ADD_COMPONENT]) RenderAddComponent();


	ImGui::Render();
}

void ComponentEngine::UIMaanger::RenderMainMenu()
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

		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Scene Manager", NULL, &m_open[SCENE]);
			ImGui::MenuItem("Add Component", NULL, &m_open[ADD_COMPONENT]);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void ComponentEngine::UIMaanger::RenderFPSCounter()
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

void ComponentEngine::UIMaanger::RenderScene()
{
	if (ImGui::Begin("Scene Manager", &m_open[SCENE], ImGuiWindowFlags_NoCollapse))
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
						ImGui::PushID(1); // Button scope 1
						bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>();
						if (!hasIndestructable && ImGui::SmallButton("X"))
						{
							m_current_scene_focus.entity->Destroy();
							m_current_scene_focus.entity = nullptr;
							m_current_scene_focus.component = nullptr;
						}
						else
						{
							if(!hasIndestructable)ImGui::SameLine();
							ImGui::Text("Entity");
							ImGui::NextColumn();
							ImGui::Text(m_current_scene_focus.entity->GetName().c_str());
							ImGui::Columns(1);
							ImGui::Separator();
						}
						ImGui::PopID();
					}
					{// Body
						RenderEntity(m_current_scene_focus.entity);
					}
				}

				if (m_current_scene_focus.component != nullptr)
				{
					// Title
					{
						ImGui::Columns(2);
						ImGui::Separator();
						ImGui::PushID(2); // Button scope 2
						bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>();
						if (!hasIndestructable && ImGui::SmallButton("X"))
						{
							m_current_scene_focus.entity->RemoveComponent(m_current_scene_focus.component->GetComponentPtr());
							m_current_scene_focus.component = nullptr;
						}
						else
						{
							if (!hasIndestructable)ImGui::SameLine();
							ImGui::Text("Component");
							ImGui::NextColumn();
							ImGui::Text(m_current_scene_focus.component->GetName().c_str());
							ImGui::Columns(1);
							ImGui::Separator();
						}
						ImGui::PopID();
					}
					{ // Body
						RenderComponent();
					}
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

void ComponentEngine::UIMaanger::RenderEntityTreeNode(Entity * entity)
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
			if(wrapper.GetID() != m_indestructable_component_id)RenderComponentTreeNode(entity, wrapper);
		});
		ImGui::TreePop();
	}
}

void ComponentEngine::UIMaanger::RenderEntity(Entity * entity)
{
	ImGui::Text("Component Count:%i", 0);
	bool clicked = ImGui::Button("Add Component");
	if (clicked)
	{
		m_open[ADD_COMPONENT] = true;
	}
}

void ComponentEngine::UIMaanger::RenderComponentTreeNode(Entity* entity, BaseComponentWrapper & wrapper)
{
	ImGui::TreeNodeEx("Component", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "%s", wrapper.GetName().c_str());

	if (ElementClicked())
	{
		m_current_scene_focus.entity = entity;
		m_current_scene_focus.component = &wrapper;
	}

}

void ComponentEngine::UIMaanger::RenderComponent()
{
	if (m_current_scene_focus.entity == nullptr || m_current_scene_focus.component == nullptr)return;

	m_current_scene_focus.entity->ForEach<UI>([this](Entity* entity, BaseComponentWrapper& wrapper, UI* ui)
	{
		if (m_current_scene_focus.component->GetComponentPtr() == wrapper.GetComponentPtr())
		{
			ui->Display();
		}
	});

}

void ComponentEngine::UIMaanger::RenderAddComponent()
{
	if (ImGui::Begin("Add Component", &m_open[ADD_COMPONENT], ImGuiWindowFlags_NoCollapse))
	{
		if (m_current_scene_focus.entity == nullptr) // No entity selected
		{
			ImGui::Text("No Entity Selected");
		}
		else // Entity selected
		{
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
			static const char* item_current = items[0];
			if (ImGui::BeginCombo("Component", item_current))
			{
				for (int n = 0; n < 4; n++)
				{
					bool is_selected = (item_current == items[n]);
					if (ImGui::Selectable(items[n], is_selected))
						item_current = items[n];
					if (is_selected)
						ImGui::SetItemDefaultFocus();   
				}
				ImGui::EndCombo();
			}
			bool selected = ImGui::Button("Add");
			if (selected)
			{
				m_open[ADD_COMPONENT] = false;
			}

		}
	}
	ImGui::End();
}

bool ComponentEngine::UIMaanger::ElementClicked()
{
	return ImGui::IsItemHovered() && ImGui::IsMouseClicked(0);
}
