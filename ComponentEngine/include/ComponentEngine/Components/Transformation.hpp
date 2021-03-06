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
#include <ComponentEngine\Components\IO.hpp>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace ComponentEngine
{
	class Transformation : public UI, public IO
	{

		glm::mat4 m_local_mat4;

	public:
		Transformation(enteez::Entity* entity)
		{
			//MsgRecive<OnComponentEnter<Transformation>>
			m_entity = entity;
			m_local_mat4 = glm::mat4(1.0f);
			m_mat4 = new glm::mat4(1.0f);
			Send(m_entity, m_entity, OnComponentEnter<Transformation>(this));
		}

		~Transformation();
		virtual void Display();
		static bool DisplayTransform(glm::mat4& mat4);

		virtual void Load(pugi::xml_node& node);
		virtual void Save(pugi::xml_node& node);

		void Translate(glm::vec3 translation);
		void SetWorldX(float x);
		void SetWorldY(float y);
		void SetWorldZ(float z);

		void SetWorld(float x, float y, float z);

		void MoveWorldX(float x);
		void MoveWorldY(float y);
		void MoveWorldZ(float z);

		void MoveLocalX(float x);
		void MoveLocalY(float y);
		void MoveLocalZ(float z);

		void SetLocalMat4(glm::mat4 mat4, bool updatePhysics = false);
		void SetWorldMat4(glm::mat4 mat4, bool updatePhysics = false);

		glm::mat4& GetLocalMat4();
		glm::mat4& GetMat4();

		glm::mat4 GetParentWorldMat4(); 

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
		glm::mat4& Get();
		enteez::Entity* GetParent();
		std::vector<Transformation*> GetChildren();
		bool HasChildren();

		enteez::Entity* GetEntity();
		static enteez::BaseComponentWrapper* EntityHookDefault(enteez::Entity& entity);

		friend class Mesh;
	private:


		void AddChild(Transformation* trans);
		void RemoveChild(Transformation* trans);
		void PushToPositionArray(bool updatePhysics = true);
		enteez::Entity* m_entity;
		int m_index;
		glm::mat4* m_mat4;
		enteez::Entity* m_parent = nullptr;
		std::vector<Transformation*> m_children;
	};
}