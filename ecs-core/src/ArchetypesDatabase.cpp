#include "ArchetypesDatabase.h"

std::unordered_map<size_t, ecs::ArchetypesDatabase::archetype_set> ecs::ArchetypesDatabase::s_archetypesMap;
std::unordered_map<ecs::entity_id, size_t> ecs::ArchetypesDatabase::s_entitiesArchetypeHashesMap;

ecs::ArchetypesDatabase::archetype_set::archetype_set(const ecs::archetype& archetype)
{
    m_archetype = std::move(archetype);

    for (auto componentIt = m_archetype.begin(); componentIt != m_archetype.end(); ++componentIt)
    {
        ecs::component_data componentData;
        if (!ecs::ComponentsDatabase::TryGetComponentData(*componentIt, componentData))
        {
            throw std::invalid_argument("Component not found in the database. Call RegisterComponent() first.");
        }
        
        m_componentArraysMap.emplace(componentData.serial(), std::make_shared<packed_component_array_t>(componentData.hash(),
            componentData.data_size(), componentData.serial(), componentData.initial_capacity()));
    }
}

size_t ecs::ArchetypesDatabase::archetype_set::add_entity(entity_id entity)
{
    size_t entityIndex = 0;
    for (auto packedArrayIt = m_componentArraysMap.begin(); packedArrayIt != m_componentArraysMap.end(); ++packedArrayIt)
    {
        entityIndex = packedArrayIt->second->size();
        packedArrayIt->second->add_component();
    }

    m_entityToIndexMap[entity] = entityIndex;
    m_indexToEntityMap[entityIndex] = entity;
    return entityIndex;
}

size_t ecs::ArchetypesDatabase::archetype_set::get_entity_index(entity_id entity) const
{
    return m_entityToIndexMap.at(entity);
}

bool ecs::ArchetypesDatabase::archetype_set::try_get_entity_index(entity_id entity, size_t& index) const
{
    auto optionalIndex = m_entityToIndexMap.find(entity);
    if (optionalIndex != m_entityToIndexMap.end())
    {
        index = optionalIndex->second;
        return true;
    }

    return false;
}

void* ecs::ArchetypesDatabase::archetype_set::get_component_at_index(const type_hash_t componentHash, const size_t index) const
{
    const ecs::component_id componentID = ecs::ComponentsDatabase::GetComponentID(componentHash);
    std::shared_ptr<packed_component_array_t> packedArray = m_componentArraysMap.at(componentID);
    if (packedArray.get() != nullptr)
    {
        return packedArray->get_component(index);
    }

    throw std::runtime_error("Found a null packed_component_array");
}

void ecs::ArchetypesDatabase::archetype_set::remove_entity(ecs::entity_id entity)
{
    auto optionalIndex = m_entityToIndexMap.find(entity);
    if (optionalIndex == m_entityToIndexMap.end())
    {
        return;
    }

    const size_t index = optionalIndex->second;
    const size_t lastIndex = m_entityToIndexMap.size() - 1;
    const entity_id lastEntity = m_indexToEntityMap[lastIndex];
    for (auto packedArrayIt = m_componentArraysMap.begin(); packedArrayIt != m_componentArraysMap.end(); ++packedArrayIt)
    {
        packedArrayIt->second->delete_at(index);
    }

    m_entityToIndexMap[lastEntity] = index;
    m_indexToEntityMap[index] = lastEntity;
    m_indexToEntityMap.erase(lastIndex);
}

void ecs::ArchetypesDatabase::AddEntity(ecs::entity_id entity, std::initializer_list<ecs::component_data> componentsData)
{
    const size_t archetypeHash = CalculateArchetypeHash(componentsData);
    AddEntity(entity, archetype(componentsData));
}

void ecs::ArchetypesDatabase::AddEntity(entity_id entity, const ecs::archetype& archetype)
{
    const size_t archetypeHash = CalculateArchetypeHash(archetype);
    auto optionalArchetypeSet = s_archetypesMap.find(archetypeHash);
    if (optionalArchetypeSet == s_archetypesMap.end())
    {
        // create and add new archetype set 
        s_archetypesMap.emplace(archetypeHash, archetype_set(archetype));
        // @todo can we avoid a second lookup here?
        s_archetypesMap[archetypeHash].add_entity(entity);
    }
    else
    {
        // just add the entity
        optionalArchetypeSet->second.add_entity(entity);
    }

    // associate the entity to that archetype hash.
    s_entitiesArchetypeHashesMap[entity] = archetypeHash;
}

void* ecs::ArchetypesDatabase::GetComponent(entity_id entity, const type_hash_t componentHash)
{
    const size_t archetypeHash = s_entitiesArchetypeHashesMap.at(entity);
    archetype_set& set = s_archetypesMap.at(archetypeHash);
    size_t entityIndex = set.get_entity_index(entity);
    return set.get_component_at_index(componentHash, entityIndex);
}

void ecs::ArchetypesDatabase::RemoveEntity(entity_id entity)
{
    auto optionalArchetypeHash = s_entitiesArchetypeHashesMap.find(entity);
    if (optionalArchetypeHash != s_entitiesArchetypeHashesMap.end())
    {
        auto optionalArchetypeSet = s_archetypesMap.find(optionalArchetypeHash->second);
        if (optionalArchetypeSet != s_archetypesMap.end())
        {
            optionalArchetypeSet->second.remove_entity(entity);
        }

        s_entitiesArchetypeHashesMap.erase(entity);
    }
}

void ecs::ArchetypesDatabase::Reset()
{
    s_archetypesMap.clear();
}