#include "Entity.h"
#include "ComponentsDatabase.h"

using namespace ecs;

entity_t::entity_t()
{
    m_componentsSignature.reset();
}

void entity_t::add_component(const component_id componentID)
{
    m_componentsSignature.set(componentID, true);
}

void entity_t::remove_component(const component_id componentID)
{
    m_componentsSignature.set(componentID, false);
}

bool entity_t::has_component(const component_id componentID) const
{
    return m_componentsSignature.test(componentID);
}