#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <ComponentEngine\Components\MsgRecive.hpp>
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
		Transformation()
		{
			m_mat4 = new glm::mat4(1.0f);
			m_origional = true;
			m_position = glm::vec3();
			m_rotation = glm::vec3();
			m_scale = glm::vec3();
		}
		Transformation(glm::mat4* mat4)
		{
			m_mat4 = mat4;
			m_origional = false;
			m_position = glm::vec3();
			m_rotation = glm::vec3();
			m_scale = glm::vec3();
		}
		~Transformation()
		{
			if (m_origional)delete m_mat4;
		}
		virtual void ReciveMessage(enteez::Entity* sender, const TransformationPtrRedirect& message);
		virtual void Display();
		void Translate(glm::vec3 translation);
		void Scale(glm::vec3 scale);
		void Rotate(glm::vec3 axis, float angle);
		void Rotate(glm::vec3 angles);
		void SetParent(Transformation* parent);
		// We are not responsible for the new memory. Needs 3rd part memory managment
		void MemoryPointTo(glm::mat4* new_mat4, bool transfer_old_data = false);
		glm::mat4& Get();
		Transformation* GetParent();


		static void EntityHook(enteez::Entity& entity, pugi::xml_node& component_data);


	private:
		glm::mat4* m_mat4;
		glm::vec3 m_position;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;
		Transformation* m_parent = nullptr;
		bool m_origional;
	};
}