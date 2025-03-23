#include "World.h"
#include "ArchetypesRegistry.h"
#include "ComponentsRegistry.h"
#include "Entity.h"


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
	const entity_id id = m_entityIDGenerator.GenerateNewUniqueID();
	m_archetypesRegistry->AddEntity(id);
	return id;
}

ecs::EntityHandle ecs::World::GetEntity(entity_id id)
{
	std::weak_ptr<World> weakPtrToThis = shared_from_this();
	const archetype_id archetypeID = m_archetypesRegistry->GetArchetypeID(id);
	return EntityHandle(weakPtrToThis, id, archetypeID);
}

ecs::ComponentsRegistry* ecs::World::GetComponentsRegistry() const
{
	return m_componentsRegistry.get();
}

ecs::ArchetypesRegistry* ecs::World::GetArchetypesRegistry() const
{
	return m_archetypesRegistry.get();
}

void ecs::World::Update(ecs::real_t deltaTime)
{
	std::shared_ptr<World> sharedSelf = shared_from_this(); 
	for (auto systemPair : m_registeredSystems)
	{
		systemPair.second->Update(sharedSelf, deltaTime);
	}
}
