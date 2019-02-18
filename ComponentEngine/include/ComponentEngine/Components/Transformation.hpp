#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>


#include <ComponentEngine\Components\MsgRecive.hpp>
#include <ComponentEngine\Components\MsgSend.hpp>
#include <ComponentEngine\Components\ComponentMessages.hpp>
#include <ComponentEngine\Components\UI.hpp>

namespace enteez
{
	class Entity;
}

namespace pugi
{
	class xml_node;
}

namespace ComponentEngine
{
	class Transformation : public UI, public MsgRecive<TransformationPtrRedirect>
	{
	public:
		Transformation(enteez::Entity* entity)
		{
			//MsgRecive<OnComponentEnter<Transformation>>
			m_entity = entity;
			m_local_mat4 = glm::mat4(1.0f);
			m_mat4 = new glm::mat4(1.0f);
			m_origional = true;
			Send(m_entity, OnComponentEnter<Transformation>(this));
		}
		Transformation(enteez::Entity* entity, glm::mat4* mat4)
		{
			m_local_mat4 = glm::mat4(1.0f);
			m_entity = entity;
			m_mat4 = mat4;
			m_origional = false;
			//Send(m_entity, OnComponentEnter<Transformation>());
		}
		~Transformation()
		{
			Send(m_entity, OnComponentExit<Transformation>(this));
			if (m_origional)delete m_mat4;
		}
		virtual void ReciveMessage(enteez::Entity* sender, TransformationPtrRedirect& message);
		virtual void Display();
		void Translate(glm::vec3 translation);
		void SetWorldX(float x);
		void SetWorldY(float y);
		void SetWorldZ(float z);

		void MoveWorldX(float x);
		void MoveWorldY(float y);
		void MoveWorldZ(float z);

		void MoveLocalX(float x);
		void MoveLocalY(float y);
		void MoveLocalZ(float z);

		float GetWorldX();
		float GetWorldY();
		float GetWorldZ();
		glm::vec3 GetLocalPosition();
		glm::vec3 GetWorldPosition();
		void Scale(glm::vec3 scale);

		void RotateWorldX(float x);
		void RotateWorldY(float y);
		void RotateWorldZ(float z);

		void Rotate(glm::vec3 axis, float angle);
		void Rotate(glm::vec3 angles);
		void SetParent(enteez::Entity* parent);
		// We are not responsible for the new memory. Needs 3rd part memory managment
		void MemoryPointTo(glm::mat4* new_mat4, bool transfer_old_data = false);
		glm::mat4& Get();
		enteez::Entity* GetParent();
		std::vector<Transformation*> GetChildren();
		bool HasChildren();

		enteez::Entity* GetEntity();
		static void EntityHookXML(enteez::Entity& entity, pugi::xml_node& component_data);

		friend class Mesh;
	private:
		void AddChild(Transformation* trans);
		void RemoveChild(Transformation* trans);
		void PushToPositionArray();
		enteez::Entity* m_entity;
		glm::mat4* m_mat4;
		glm::mat4 m_local_mat4;
		enteez::Entity* m_parent = nullptr;
		std::vector<Transformation*> m_children;
		bool m_origional;
	};
}