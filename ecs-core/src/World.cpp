#include "World.h"
#include "ArchetypesRegistry.h"
#include "ComponentsRegistry.h"

using namespace ecs;

World::World()
{
	m_componentsRegistry = std::make_shared<ComponentsRegistry>();
	m_archetypesRegistry = std::make_shared<ArchetypesRegistry>(m_componentsRegistry);
}

World::~World()
{
	m_archetypesRegistry.reset();
	m_componentsRegistry.reset();
}

entity_id World::CreateEntity()
{
	const entity_id id = m_entityIDGenerator.GenerateNewUniqueID();
	m_archetypesRegistry->AddEntity(id);
	return id;
}

EntityHandle World::GetEntity(entity_id id)
{
	std::weak_ptr<World> weakPtrToThis = shared_from_this();
	const archetype_id archetypeID = m_archetypesRegistry->GetArchetypeID(id);
	return EntityHandle(weakPtrToThis, id, archetypeID);
}
