#pragma once

#include <ComponentEngine\Components\Logic.hpp>
#include <ComponentEngine\Components\UI.hpp>
#include <btBulletDynamicsCommon.h>

namespace enteez
{
	class Entity;
	class BaseComponentWrapper;
}

namespace ComponentEngine
{
	class ICollisionShape : public Logic, public UI
	{
	public:
		ICollisionShape(enteez::Entity* m_entity);
		virtual void Update(float frame_time);

		btCollisionShape * GetCollisionShape();
	protected:
		enteez::Entity* m_entity = nullptr;
		btCollisionShape * m_colShape;

	};
}