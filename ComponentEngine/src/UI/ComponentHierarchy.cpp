#include <ComponentEngine\UI\ComponentHierarchy.hpp>
#include <ComponentEngine\UI\UIManager.hpp>
#include <ComponentEngine\Engine.hpp>

#include <enteez\Entity.hpp>

#include <mutex>

using namespace ComponentEngine;
using namespace enteez;

ComponentEngine::ComponentHierarchy::ComponentHierarchy(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::ComponentHierarchy::Contents()
{
	
	EntityManager& em = m_engine->GetEntityManager();

	static bool v_borders = false;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	{


		ImGui::BeginChild("Child2", ImVec2(0.0f, 0.0f), false);
		{
			{
				if (m_manager->GetCurrentSceneFocus().entity != nullptr)
				{
					bool entityBeenDestroyed = false;
					// Title
					{
						ImGui::Columns(2, NULL, v_borders);
						ImGui::Separator();
						ImGui::PushID(m_manager->GetCurrentSceneFocus().entity); // Button scope 1

						entityBeenDestroyed = ImGui::SmallButton("X");

						ImGui::SameLine();
						ImGui::Text("Entity");
						ImGui::NextColumn();

						m_manager->EdiableText(m_manager->GetCurrentSceneFocus().entity->GetName(), m_manager->GetCurrentSceneFocus().entity_temp_name, 20, true);

						ImGui::Columns(1);
						ImGui::Separator();


						if (entityBeenDestroyed)
						{
							// Loop through all children and destroy them
							DestroyEntity(m_manager->GetCurrentSceneFocus().entity);

							m_manager->ResetSceneFocus();
						}
						ImGui::PopID();



					}
					if (!entityBeenDestroyed)
					{// Body
						RenderComponentTitleBar(m_manager->GetCurrentSceneFocus().entity);
						RenderComponentList(m_manager->GetCurrentSceneFocus().entity);
					}

					if (!entityBeenDestroyed)
					{ // Components
						m_manager->GetCurrentSceneFocus().entity->ForEach<UI>([this](Entity* entity, BaseComponentWrapper& wrapper, UI* ui)
						{
							bool componentBeenDestroyed = false;
							// Title
							{
								ImGui::Columns(2);
								ImGui::Separator();
								ImGui::PushID(wrapper.GetComponentPtr());

								bool hasIndestructable = wrapper.GetName() == "Transformation";

								componentBeenDestroyed = !hasIndestructable && ImGui::SmallButton("X");

								if (componentBeenDestroyed)
								{
									m_manager->GetCurrentSceneFocus().entity->RemoveComponent(wrapper.GetComponentPtr());
								}
								else
								{
									if (!hasIndestructable)ImGui::SameLine();
									ImGui::Text("Component");
									ImGui::NextColumn();
									ImGui::Text(wrapper.GetName().c_str());
									ImGui::Columns(1);
									ImGui::Separator();
								}
								ImGui::PopID();
							}
							if (!componentBeenDestroyed)
							{ // Body
								ui->Display();
							}
						});
					}
				}
			}
			ImGui::EndChild();
		}
	}
	ImGui::Columns(1);
	ImGui::PopStyleVar();
}

void ComponentEngine::ComponentHierarchy::DestroyEntity(Entity * entity)
{
	Transformation& transformation = entity->GetComponent<Transformation>();
	if (transformation.HasChildren())
	{
		for (auto t : transformation.GetChildren())
		{
			DestroyEntity(t->GetEntity());
		}
	}
	entity->Destroy();
}

void ComponentEngine::ComponentHierarchy::RenderComponentTitleBar(Entity * entity)
{
	ImGui::Text("Component Count:%i", entity->GetComponentCount());
}

void ComponentEngine::ComponentHierarchy::RenderComponentList(Entity * entity)
{
	if (ImGui::Button("Add Component"))
		ImGui::OpenPopup("AddComponentPopup");
	if (ImGui::BeginPopup("AddComponentPopup"))
	{
		if (m_manager->GetCurrentSceneFocus().entity == nullptr) // No entity selected
		{
			ImGui::Text("No Entity Selected");
		}
		else // Entity selected
		{
			static std::string item_current = "Renderer";

			auto componentRegister = m_engine->GetComponentRegister();

			if (ImGui::BeginCombo("Component", item_current.c_str()))
			{
				for (auto it : componentRegister)
				{
					if (it.second != nullptr)
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
				auto it = componentRegister.find(item_current);
				if (it != componentRegister.end())
				{
					if (it->second != nullptr)
					{
						// In-case we replace the current component with a new one, we want to forget the old one now
						it->second(*m_manager->GetCurrentSceneFocus().entity);
					}
				}
				ImGui::CloseCurrentPopup();
			}

		}


		ImGui::EndPopup();
	}
}
