#include "ComponentArray.h"
#include <typeinfo>
#include <iostream>

using namespace ecs;

component_array_base::component_array_base()
{
    m_componentSize = 1;
    m_maxNumComponents = 1;
    m_componentType = typeid(IComponent).hash_code();

    initialize_containers();
}

component_array_base::component_array_base(type_hash_t componentType, size_t sizeOfComponent, 
    size_t maxNumComponents)
{
    m_componentSize = sizeOfComponent;
    m_maxNumComponents = maxNumComponents;
    m_componentType = componentType;

    initialize_containers();
}

component_array_base::byte* component_array_base::allocate_component(const entity_id entityID)
{
    if (m_indicesMap.find(entityID) != m_indicesMap.end())
    {
        throw std::invalid_argument("Component of given type already added to entity.");
    }

    size_t nextFreeComponentIndex = m_indicesMap.size();
    if (m_freeIndices.size() > 0)
    {
        nextFreeComponentIndex = m_freeIndices.back();
        m_freeIndices.pop_back();
    }

    m_indicesMap[entityID] = nextFreeComponentIndex;
    return &m_data[nextFreeComponentIndex * m_componentSize];
}

component_array_base::byte* component_array_base::get_byte_ptr(const entity_id entityID) const
{
    const size_t cIndex = m_indicesMap.at(entityID);
    const size_t byteIndex = cIndex * m_componentSize;
    return &m_data[byteIndex];
}

component_array_base::byte* component_array_base::find_byte_ptr(const entity_id entityID) const
{
    auto optionalIndex = m_indicesMap.find(entityID);
    if (optionalIndex == m_indicesMap.end())
    {
        return nullptr;
    }

    return get_byte_ptr(entityID);
}

bool component_array_base::free_component(const entity_id entityID)
{
    auto optionalIdx = m_indicesMap.find(entityID);
    if (optionalIdx == m_indicesMap.end())
    {
        return false;
    }

    const size_t componentIdx = optionalIdx->second;
    m_freeIndices.push_back(componentIdx);

    m_indicesMap.erase(entityID);
    return true;
}

void component_array_base::initialize_containers()
{
    m_data = std::make_unique<byte[]>(m_maxNumComponents * m_componentSize);
    m_indicesMap.reserve(m_maxNumComponents);
}