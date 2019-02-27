#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include<ComponentEngine\TemplateType.hpp>
#include <glm/glm.hpp>

namespace ComponentEngine
{
	struct ParticleSystemVisibility
	{
		bool visible;
	};
	struct OnCollisionEnter
	{
		enteez::Entity* collider;
	};
	struct OnCollisionExit
	{
		enteez::Entity* collider;
	};
	struct OnCollision
	{
		enteez::Entity* collider;
	};

	enum CollisionRecordingState
	{
		Begin,
		End
	};

	struct CollisionRecording
	{
		CollisionRecordingState state;
	};

	struct CollisionEvent
	{
		enteez::Entity* collider;
	};

	struct TransformationChange
	{
		glm::mat4 mat;
	};

	struct RenderStatus
	{
		RenderStatus(bool should_renderer) : should_renderer(should_renderer) {}
		bool should_renderer;
	};

	struct TransformationPtrRedirect
	{
		TransformationPtrRedirect(glm::mat4* mat_ptr) : mat_ptr(mat_ptr) {}
		glm::mat4* mat_ptr;
	};

	template<class T>
	class OnComponentExit : public TemplateType<T>
	{
	public:
		OnComponentExit(T* component);
		T& GetComponent();
	private:
		T * m_component;
	};
	template<class T>
	inline OnComponentExit<T>::OnComponentExit(T * component)
	{
		m_component = component;
	}
	template<class T>
	inline T & OnComponentExit<T>::GetComponent()
	{
		return *m_component;
	}

	template<class T>
	class OnComponentEnter : public TemplateType<T>
	{
	public:
		OnComponentEnter(T* component);
		T& GetComponent();
	private:
		T * m_component;
	};
	template<class T>
	inline OnComponentEnter<T>::OnComponentEnter(T * component)
	{
		m_component = component;
	}
	template<class T>
	inline T & OnComponentEnter<T>::GetComponent()
	{
		return *m_component;
	}

	template<class T>
	class OnComponentChange : public TemplateType<T>
	{
	public:
		OnComponentChange(T* component);
		T& GetComponent();
	private:
		T * m_component;
	};
	template<class T>
	inline OnComponentChange<T>::OnComponentChange(T * component)
	{
		m_component = component;
	}
	template<class T>
	inline T & OnComponentChange<T>::GetComponent()
	{
		return *m_component;
	}
}