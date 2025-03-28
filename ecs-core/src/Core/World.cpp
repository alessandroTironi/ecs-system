#include "Core/World.h"
#include "Core/ArchetypesRegistry.h"
#include "Core/ComponentsRegistry.h"
#include "Core/Entity.h"

ecs::World::~World()
{
	m_archetypesRegistry.reset();
	m_componentsRegistry.reset();
}

void ecs::World::Initialize()
{
	m_archetypesRegistry = std::make_shared<ArchetypesRegistry>(shared_from_this());
	m_componentsRegistry = std::make_shared<ComponentsRegistry>();
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

std::shared_ptr<ecs::ComponentsRegistry> ecs::World::GetComponentsRegistry() const
{
	return m_componentsRegistry;
}

std::shared_ptr<ecs::ArchetypesRegistry> ecs::World::GetArchetypesRegistry() const
{
	return m_archetypesRegistry;
}

void ecs::World::Update(ecs::real_t deltaTime)
{
	std::shared_ptr<World> sharedSelf = shared_from_this(); 
	for (auto systemPair : m_registeredSystems)
	{
		systemPair.second->Update(sharedSelf, deltaTime);
	}
}
