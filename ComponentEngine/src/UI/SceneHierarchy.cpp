#include <ComponentEngine\UI\SceneHierarchy.hpp>
#include <ComponentEngine\UI\UIManager.hpp>
#include <ComponentEngine\Engine.hpp>

#include <EnteeZ\Entity.hpp>

#include <mutex>

using namespace ComponentEngine;
using namespace enteez;

ComponentEngine::SceneHierarchy::SceneHierarchy(const char * title, ImGuiWindowFlags flags, UIDisplayFlags displayFlags, bool open) :
	UIBase(title, flags, displayFlags, open)
{

}

void ComponentEngine::SceneHierarchy::Contents()
{
	EntityManager& em = m_engine->GetEntityManager();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

	{

		ImGui::BeginChild("Child1", ImVec2(0.0f, 0.0f), false);
		{

			bool open = ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick);

			AddEntityDialougeMenu(nullptr);

			if (UIManager::ElementClicked())
			{
				m_manager->ResetSceneFocus();
				m_manager->GetCurrentSceneFocus().entity = nullptr;
			}

			const ImGuiPayload* payload = nullptr;
			if (UIManager::DropTarget("Entity", payload))
			{
				IM_ASSERT(payload->DataSize == sizeof(Entity*));
				Entity* child = *(Entity**)payload->Data;

				child->GetComponent<Transformation>().SetParent(nullptr);
			}

			if (open)
			{

				for (auto entity : em.GetEntitys())
				{
					if (entity->GetComponent<Transformation>().GetParent() == nullptr)
					{
						RenderEntityTreeNode(entity);
					}
				}


				ImGui::TreePop();
			}
			ImGui::EndChild();
		}
	}

	ImGui::Columns(1);
	ImGui::PopStyleVar();

}

void ComponentEngine::SceneHierarchy::AddEntityDialougeMenu(Entity* parent)
{
	if (ImGui::BeginPopupContextItem("Add Entity Menu"))
	{
		if (ImGui::Selectable("Add Entity"))
		{
			AddEntity(parent);
		}
		ImGui::EndPopup();
	}
}

void ComponentEngine::SceneHierarchy::AddEntity(Entity * parent)
{
	EntityManager& em = m_engine->GetEntityManager();
	enteez::Entity* entity = em.CreateEntity("New Entity");
	Transformation::EntityHookDefault(*entity);
	Transformation& trans = entity->GetComponent<Transformation>();
	trans.SetParent(parent);
}

void ComponentEngine::SceneHierarchy::RenderEntityTreeNode(Entity * entity)
{
	ImGui::PushID(entity);
	Transformation& entityTeansformation = entity->GetComponent<Transformation>();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (!entityTeansformation.HasChildren())
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}
	if (entity == m_manager->GetCurrentSceneFocus().entity)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	bool open = ImGui::TreeNodeEx("GameObject", flags, "%s", entity->GetName().c_str());

	AddEntityDialougeMenu(entity);

	if (UIManager::ElementClicked())
	{
		m_manager->ResetSceneFocus();
		m_manager->GetCurrentSceneFocus().entity = entity;
		//ResetSceneFocusComponent();
	}

	UIManager::DropPayload("Entity", entity->GetName().c_str(), &entity, sizeof(Entity*));



	const ImGuiPayload* payload = nullptr;
	if (UIManager::DropTarget("Entity", payload))
	{
		IM_ASSERT(payload->DataSize == sizeof(Entity*));
		Entity* child = *(Entity**)payload->Data;

		bool notLooped = true;
		Entity* currentParent = entity->GetComponent<Transformation>().GetParent();
		while (currentParent != nullptr)
		{
			if (currentParent == child)
			{
				notLooped = false;
				break;
			}
			currentParent = currentParent->GetComponent<Transformation>().GetParent();
		}

		if (notLooped)
		{
			child->GetComponent<Transformation>().SetParent(entity);
		}

	}


	if (open)
	{

		std::vector<Transformation*> children = entityTeansformation.GetChildren();
		for (auto child : children)
		{
			ImGui::PushID(child->GetEntity());

			RenderEntityTreeNode(child->GetEntity());

			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}
