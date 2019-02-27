#include <ComponentEngine\Components\ICollisionShape.hpp>

#include <EnteeZ\EnteeZ.hpp>

ComponentEngine::ICollisionShape::ICollisionShape(enteez::Entity * entity)
{
	m_entity = entity;
	m_entity->ForEach<ICollisionShape>([&](enteez::Entity* entity, ICollisionShape& recive)
	{
		entity->RemoveComponent(&recive);
	});
}

void ComponentEngine::ICollisionShape::Update(float frame_time)
{
}

btCollisionShape * ComponentEngine::ICollisionShape::GetCollisionShape()
{
	return m_colShape;
}
