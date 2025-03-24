#include "Entity.h"
#include "ComponentsRegistry.h"
#include "World.h"

using namespace ecs;

EntityHandle::EntityHandle()
{
    
}

EntityHandle::EntityHandle(std::weak_ptr<World> world, entity_id id, archetype_id archetypeID)
    : m_world(world), m_id(id), m_archetypeID(archetypeID)
{}


void EntityHandle::AddComponent(component_id componentID)
{
    if (ArchetypesRegistry* archetypesRegistry = GetArchetypesRegistry())
    {
        archetypesRegistry->AddComponent(m_id, componentID);
    }
}

void* EntityHandle::GetComponent(component_id componentID) const
{
    if (ArchetypesRegistry* archetypesRegistry = GetArchetypesRegistry())
    {
        return archetypesRegistry->GetComponent(m_id, componentID);
    }

    throw std::out_of_range("Component not found");
}

void * EntityHandle::FindComponent(component_id componentID) const noexcept
{
    if (ArchetypesRegistry* archetypesRegistry = GetArchetypesRegistry())
    {
        return archetypesRegistry->FindComponent(m_id, componentID);
    }

    return nullptr;
}

void EntityHandle::RemoveComponent(component_id componentID)
{
    if (ArchetypesRegistry* archetypesRegistry = GetArchetypesRegistry())
    {
        archetypesRegistry->RemoveComponent(m_id, componentID);
    }
}

ArchetypesRegistry* EntityHandle::GetArchetypesRegistry() const
{
    if (!m_world.expired())
    {
        return m_world.lock()->GetArchetypesRegistry();
    }

    return nullptr;
}

ComponentsRegistry* EntityHandle::GetComponentsRegistry() const
{
    if (!m_world.expired())
    {
        return m_world.lock()->GetComponentsRegistry();
    }

    return nullptr;
}
