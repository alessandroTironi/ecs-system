#include "Entity.h"
#include "ComponentsRegistry.h"

using namespace ecs;

EntityHandle::EntityHandle()
{
    
}

EntityHandle::EntityHandle(World* world, entity_id id, archetype_id archetypeID)
{

}


void EntityHandle::AddComponent(component_id componentID)
{
    throw std::runtime_error("Not implemented");
}

void* EntityHandle::GetComponent(component_id componentID)
{
    throw std::runtime_error("Not implemented");
}

void EntityHandle::RemoveComponent(component_id componentID)
{
    throw std::runtime_error("Not implemented");
}
