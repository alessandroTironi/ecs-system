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


void* EntityHandle::GetComponent(component_id componentID)
{
    throw std::runtime_error("Not implemented");
}

void EntityHandle::RemoveComponent(component_id componentID)
{
    throw std::runtime_error("Not implemented");
}
