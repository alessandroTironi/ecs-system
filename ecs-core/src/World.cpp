#include "World.h"
#include "ArchetypesRegistry.h"
#include "ComponentsRegistry.h"

ecs::World::World()
{
	m_componentsRegistry = std::make_shared<ComponentsRegistry>();
	m_archetypesRegistry = std::make_shared<ArchetypesRegistry>(m_componentsRegistry);
}

ecs::World::~World()
{
	m_archetypesRegistry.reset();
	m_componentsRegistry.reset();
}


ecs::entity_id ecs::World::CreateEntity()
{
	throw std::runtime_error("Not implemented");
}

ecs::EntityHandle ecs::World::GetEntity(entity_id id) const
{
	throw std::runtime_error("Not implemented");
}
