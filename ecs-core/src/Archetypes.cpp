#include "Archetypes.h"

ecs::archetype::archetype()
{
    
}

ecs::archetype::archetype(std::initializer_list<ecs::component_id> components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_components = std::move(components);
}

ecs::archetype::archetype(const std::set<ecs::component_id>&& components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_components = std::move(components);
}

ecs::archetype ecs::archetype::make(std::initializer_list<component_id> components)
{
    return ecs::archetype(std::move(components));
}