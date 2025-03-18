#include "Archetypes.h"
#include <iostream>

ecs::archetype::archetype()
{
    
}

ecs::archetype::archetype(std::initializer_list<ecs::component_id> components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_componentIDs = std::move(components);
}

ecs::archetype::archetype(std::initializer_list<ecs::component_data> componentsData)
{
    for (auto componentDataIt = componentsData.begin(); componentDataIt != componentsData.end(); ++componentDataIt)
    {
        m_componentIDs.insert((*componentDataIt).serial());
    }
}

ecs::archetype::archetype(const std::set<ecs::component_id>&& components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_componentIDs = std::move(components);
}

ecs::archetype ecs::archetype::make(std::initializer_list<component_id> components)
{
    return ecs::archetype(std::move(components));
}