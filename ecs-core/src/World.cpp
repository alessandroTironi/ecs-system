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
