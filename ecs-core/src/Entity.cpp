#include "Entity.h"
#include "ComponentsDatabase.h"

using namespace ecs;

entity_t::entity_t()
{
    m_componentsSignature.reset();
}

void entity_t::add_component(const type_hash_t componentHash)
{
    const component_id serial = ComponentsDatabase::GetComponentID(componentHash);
    m_componentsSignature.set(serial, true);
}

void entity_t::remove_component(const type_hash_t componentHash)
{
    const component_id serial = ComponentsDatabase::GetComponentID(componentHash);
    m_componentsSignature.set(serial, false);
}

bool entity_t::has_component(const type_hash_t componentHash) const
{
    const component_id serial = ComponentsDatabase::GetComponentID(componentHash);
    return m_componentsSignature.test(serial);
}