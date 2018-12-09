#include <ComponentEngine\UIManager.hpp>
#include <ComponentEngine\Components\UI.hpp>

#include <imgui.h>

const unsigned int ComponentEngine::UIManager::SCENE = 0;
const unsigned int ComponentEngine::UIManager::ADD_COMPONENT = 1;

ComponentEngine::UIManager::UIManager(Engine* engine) : m_engine(engine)
{
	m_indestructable_component_id = engine->GetEntityManager().GetBaseIndex<Indestructable>();
	m_open[SCENE] = true;
	m_open[ADD_COMPONENT] = false;
	m_thread_time_update_delay = 0.0f;
}

void ComponentEngine::UIManager::Render()
{
	ImGui::NewFrame();
	//ImGui::ShowTestWindow();

	
	RenderMainMenu();
	RenderFPSCounter();
	if (m_open[SCENE]) RenderScene();
	if (m_open[ADD_COMPONENT]) RenderAddComponent();


	ImGui::Render();
}

void ComponentEngine::UIManager::RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				m_engine->GetRendererMutex().lock();
				m_engine->RequestStop();
				m_engine->GetRendererMutex().unlock();
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

void ComponentEngine::UIManager::RenderFPSCounter()
{
	ImVec2 window_pos = ImVec2(10.0f, 25.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
	bool open = true;
	if (ImGui::Begin("", &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
	{
		ImGuiIO& io = ImGui::GetIO();
		//ImGui::Text("FPS:%i  TPS:%f", 1000.0f / io.Framerate, 0.0f);

		m_thread_time_update_delay -= m_engine->GetLastThreadTime();

		if (m_thread_time_update_delay < 0.0f)
		{
			m_thread_time_update_delay = 1.0f;
			//m_thread_data[id].
			for (auto it = m_engine->m_thread_data.begin(); it != m_engine->m_thread_data.end(); it++)
			{
				if (m_thread_times_miliseconds[it->first].size() == 0)
				{
					m_thread_times_miliseconds[it->first].resize(20);
				}
				m_thread_times_fraction_last[it->first] = it->second.loop_time;

				m_thread_times_miliseconds[it->first].push_back(1000 * it->second.process_time_average);
				m_thread_times_miliseconds[it->first].erase(m_thread_times_miliseconds[it->first].begin());

			}
		}

		for (auto it = m_engine->m_thread_data.begin(); it != m_engine->m_thread_data.end(); it++)
		{
			ImGui::PlotLines(
				"", // Label
				m_thread_times_miliseconds[it->first].data(), // Line data
				m_thread_times_miliseconds[it->first].size(), // Line count
				0,
				"",
				0.0f,
				(float)(1000 / it->second.requested_ups) // Calculate the slowest the thread would have to run to meet the Updates Per Second
			);

			ImGui::SameLine();

			ImGui::Text("~%04d/s:%s", (int)(1.0f / m_thread_times_fraction_last[it->first]), it->second.name);

		}
	}
	ImGui::End();
}

void ComponentEngine::UIManager::RenderScene()
{
	static int window_height = 370;
	ImGui::SetNextWindowSize(ImVec2(420, window_height));
	//ImGui::SetNextWindowPos(ImVec2(10, 110));
	if (ImGui::Begin("Scene Manager", &m_open[SCENE], ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
	{
		EntityManager& em = m_engine->GetEntityManager();
		
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		static bool v_borders = false;
		ImGui::Columns(2, NULL, v_borders);
		{
			// Add Entity
			if (ImGui::Button("Add"))
			{
				enteez::Entity* entity = em.CreateEntity("New Entity");
				Transformation::EntityHookDefault(*entity);
			}
			ImGui::BeginChild("Child1", ImVec2((ImGui::GetWindowContentRegionWidth() / 2) , window_height - 50), false);
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
			ImGui::BeginChild("Child2", ImVec2((ImGui::GetWindowContentRegionWidth() / 2), window_height - 50), false);
			{
				{
					if (m_current_scene_focus.entity != nullptr)
					{
						// Title
						{
							ImGui::Columns(2, NULL, v_borders);
							ImGui::Separator();
							ImGui::PushID(1); // Button scope 1
							bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>();
							if (!hasIndestructable && ImGui::SmallButton("X"))
							{
								m_current_scene_focus.entity->Destroy();
								ResetSceneFocusEntity();
								ResetSceneFocusComponent();
							}
							else
							{
								if (!hasIndestructable)ImGui::SameLine();
								ImGui::Text("Entity");
								ImGui::NextColumn();

								EdiableText(m_current_scene_focus.entity->GetName(), m_current_scene_focus.entity_temp_name, 20, !hasIndestructable);

								ImGui::Columns(1);
								ImGui::Separator();
							}
							ImGui::PopID();
						}
					}
					if (m_current_scene_focus.entity != nullptr)
					{
						{// Body
							RenderEntity(m_current_scene_focus.entity);
						}
					}
				}
				
				if (m_current_scene_focus.component != nullptr)
				{
					// Title
					{
						ImGui::Columns(2);
						ImGui::Separator();
						ImGui::PushID(2); // Button scope 2
						bool hasIndestructable = m_current_scene_focus.entity->HasComponent<Indestructable>() || m_current_scene_focus.component->GetName() == "Transformation";
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
		ImGui::PopStyleVar();


	}
	ImGui::End();
}

void ComponentEngine::UIManager::RenderEntityTreeNode(Entity * entity)
{
	bool node_open = ImGui::TreeNode("GameObject", "%s", entity->GetName().c_str());


	if (ElementClicked())
	{
		ResetSceneFocusEntity();
		m_current_scene_focus.entity = entity;
		ResetSceneFocusComponent();
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

void ComponentEngine::UIManager::RenderEntity(Entity * entity)
{
	ImGui::Text("Component Count:%i", entity->GetComponentCount());
	if (ImGui::Button("Add Component"))
	{
		m_open[ADD_COMPONENT] = true;
	}
}

void ComponentEngine::UIManager::RenderComponentTreeNode(Entity* entity, BaseComponentWrapper & wrapper)
{
	ImGui::TreeNodeEx("Component", ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet, "%s", wrapper.GetName().c_str());

	if (ElementClicked())
	{
		m_current_scene_focus.entity = entity;
		m_current_scene_focus.component = &wrapper;
	}

}

void ComponentEngine::UIManager::RenderComponent()
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

void ComponentEngine::UIManager::RenderAddComponent()
{
	static int window_height = 80;
	ImGui::SetNextWindowSize(ImVec2(226, window_height));
	//ImGui::SetNextWindowPos(ImVec2(362, 25));
	if (ImGui::Begin("Add Component", &m_open[ADD_COMPONENT], ImGuiWindowFlags_NoCollapse))
	{
		if (m_current_scene_focus.entity == nullptr) // No entity selected
		{
			ImGui::Text("No Entity Selected");
		}
		else // Entity selected
		{
			static std::string item_current = "Renderer";
			if (ImGui::BeginCombo("Component", item_current.c_str()))
			{
				for (auto it : m_engine->m_component_register)
				{
					if (it.second.default_initilizer!=nullptr)
					{
						bool is_selected = (item_current == it.first);
						if (ImGui::Selectable(it.first.c_str(), is_selected))
						{
							item_current = it.first;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			bool selected = ImGui::Button("Add");
			if (selected)
			{
				auto it = m_engine->m_component_register.find(item_current);
				if (it != m_engine->m_component_register.end())
				{
					if (it->second.default_initilizer != nullptr)
					{
						// In-case we replace the current component with a new one, we want to forget the old one now
						m_current_scene_focus.component = nullptr;
						it->second.default_initilizer(*m_current_scene_focus.entity);
					}
				}
				m_open[ADD_COMPONENT] = false;
			}

		}
	}
	ImGui::End();
}

bool ComponentEngine::UIManager::ElementClicked()
{
	return ImGui::IsItemHovered() && ImGui::IsMouseClicked(0);
}

bool ComponentEngine::UIManager::EdiableText(std::string & text, char *& temp_data, int max_size, bool editable)
{
	if (temp_data == nullptr || !editable)
	{
		ImGui::Text(text.c_str());
		if (ElementClicked())
		{
			temp_data = new char[max_size + 1];
			temp_data[max_size] = '\0';
			for (int i = 0; i < max_size; i++)
			{
				if (i < text.size())
				{
					temp_data[i] = m_current_scene_focus.entity->GetName().at(i);
				}
				else
				{
					temp_data[i] = '\0';
				}
			}
		}
	}
	else
	{
		ImGui::SetKeyboardFocusHere();
		ImGui::CaptureKeyboardFromApp(true);
		ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_AlwaysInsertMode;
		bool done = ImGui::InputText("##data", temp_data, max_size, flags);
		if (done)
		{
			text = std::string(temp_data);
			delete temp_data;
			temp_data = nullptr;
			return true;
		}
	}
	return false;
}

void ComponentEngine::UIManager::ResetSceneFocusEntity()
{
	m_current_scene_focus.entity = nullptr;

	if (m_current_scene_focus.entity_temp_name != nullptr)
	{
		delete m_current_scene_focus.entity_temp_name;
		m_current_scene_focus.entity_temp_name = nullptr;
	}
}

void ComponentEngine::UIManager::ResetSceneFocusComponent()
{
	m_current_scene_focus.component = nullptr;
}
