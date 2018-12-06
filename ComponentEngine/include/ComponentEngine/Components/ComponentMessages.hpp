#pragma once

#include <EnteeZ\EnteeZ.hpp>
#include<ComponentEngine\TemplateType.hpp>
#include <glm/glm.hpp>

namespace ComponentEngine
{

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
}