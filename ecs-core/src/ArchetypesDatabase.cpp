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

const ecs::archetype& ecs::ArchetypesDatabase::GetArchetype(entity_id entity)
{
    const size_t archetypeHash = s_entitiesArchetypeHashesMap.at(entity);
    return s_archetypesMap.at(archetypeHash).get_archetype();
}

void ecs::ArchetypesDatabase::AddComponent(entity_id entity, const type_hash_t componentHash)
{
    const archetype& currentArchetype = GetArchetype(entity);
    if (currentArchetype.has_component(componentHash))
    {
        return;
    }

    archetype newArchetype = currentArchetype; // @todo possibly unnecessary copy constructor here
    newArchetype.add_component(componentHash);

    MoveEntity(entity, newArchetype);
}

void ecs::ArchetypesDatabase::MoveEntity(entity_id entity, const archetype& targetArchetype)
{
    const size_t currentArchetypeHash = s_entitiesArchetypeHashesMap.at(entity);
    archetype_set& currentSet = s_archetypesMap.at(currentArchetypeHash);
    const size_t currentIndex = currentSet.get_entity_index(entity);
    
    const size_t targetArchetypeHash = CalculateArchetypeHash(targetArchetype);
    auto optionalTargetSet = s_archetypesMap.find(targetArchetypeHash);
    if (optionalTargetSet == s_archetypesMap.end())
    {
        // The target archetype still doesn't exist, create it
        s_archetypesMap.emplace(targetArchetypeHash, archetype_set(targetArchetype));
    }

    // allocate memory for storing components of the entity in the new archetype.
    archetype_set& targetSet = s_archetypesMap.at(targetArchetypeHash);
    targetSet.add_entity(entity);
    const size_t targetIndex = targetSet.get_entity_index(entity);

    for (auto componentIt = currentSet.get_archetype().begin(); componentIt != currentSet.get_archetype().end(); ++componentIt)
    {
        const type_hash_t componentHash = *componentIt;
        void* componentPtr = currentSet.get_component_at_index(componentHash, currentIndex);
        void* targetComponentPtr = targetSet.get_component_at_index(componentHash, targetIndex);
        component_data componentData;
        if (!ComponentsDatabase::TryGetComponentData(componentHash, componentData))
        {
            throw std::runtime_error("Trying to copy a component that was not registered. Call RegisterComponent() first.");
        }

        // copy component from current to target archetype set.
        const size_t sizeOfComponent = componentData.data_size();
        std::memcpy(targetComponentPtr, componentPtr, sizeOfComponent);
    }

    // remove entity from old set
    currentSet.remove_entity(entity);

    // update entities to archetypes map
    s_entitiesArchetypeHashesMap[entity] = targetArchetypeHash;
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