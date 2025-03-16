#include "Archetypes.h"
#include <iostream>

ecs::archetype::archetype()
{
    
}

ecs::archetype::archetype(std::initializer_list<ecs::type_hash_t> components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_componentTypes = std::move(components);
}

ecs::archetype::archetype(std::initializer_list<ecs::component_data> componentsData)
{
    for (auto componentDataIt = componentsData.begin(); componentDataIt != componentsData.end(); ++componentDataIt)
    {
        m_componentTypes.insert((*componentDataIt).hash());
    }
}

ecs::archetype::archetype(const std::set<ecs::type_hash_t>&& components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_componentTypes = std::move(components);
}

ecs::archetype ecs::archetype::make(std::initializer_list<type_hash_t> components)
{
    return ecs::archetype(std::move(components));
}