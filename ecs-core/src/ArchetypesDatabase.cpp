#include "ArchetypesDatabase.h"

ecs::ArchetypesDatabase::archetype_set::archetype_set(const ecs::archetype& archetype, 
    ecs::ComponentsDatabase* componentsRegistry)
{
    m_archetype = std::move(archetype);

    for (auto componentIt = m_archetype.begin(); componentIt != m_archetype.end(); ++componentIt)
    {
        ecs::component_data componentData;
        ecs::name componentName;
        if (!componentsRegistry->TryGetComponentData(*componentIt, componentName, componentData))
        {
            throw std::invalid_argument("Component not found in the database. Call RegisterComponent() first.");
        }
        
        m_componentArraysMap.emplace(componentData.serial(), std::make_shared<packed_component_array_t>(componentData));
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

void* ecs::ArchetypesDatabase::archetype_set::get_component_at_index(const component_id componentID, const size_t index) const
{
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

    m_indexToEntityMap.erase(lastIndex);
    m_entityToIndexMap.erase(entity);
    if (entity != lastEntity)
    {
        m_entityToIndexMap[lastEntity] = index;
        m_indexToEntityMap[index] = lastEntity;
    }
}

void ecs::ArchetypesDatabase::AddEntity(ecs::entity_id entity, std::initializer_list<ecs::component_data> componentsData)
{
    const size_t archetypeHash = CalculateArchetypeHash(componentsData);
    AddEntity(entity, archetype(componentsData));
}

void ecs::ArchetypesDatabase::AddEntity(entity_id entity, const ecs::archetype& archetype)
{
    const size_t archetypeHash = CalculateArchetypeHash(archetype);
    auto optionalArchetypeSet = m_archetypesMap.find(archetypeHash);
    if (optionalArchetypeSet == m_archetypesMap.end())
    {
        // create and add new archetype set 
        m_archetypesMap.emplace(archetypeHash, archetype_set(archetype, m_componentsRegistry.get()));
        // @todo can we avoid a second lookup here?
        m_archetypesMap[archetypeHash].add_entity(entity);
    }
    else
    {
        // just add the entity
        optionalArchetypeSet->second.add_entity(entity);
    }

    // associate the entity to that archetype hash.
    m_entitiesArchetypeHashesMap[entity] = archetypeHash;
}

void* ecs::ArchetypesDatabase::GetComponent(entity_id entity, const component_id componentID)
{
    const size_t archetypeHash = m_entitiesArchetypeHashesMap.at(entity);
    archetype_set& set = m_archetypesMap.at(archetypeHash);
    size_t entityIndex = set.get_entity_index(entity);
    return set.get_component_at_index(componentID, entityIndex);
}

const ecs::archetype& ecs::ArchetypesDatabase::GetArchetype(entity_id entity)
{
    const size_t archetypeHash = m_entitiesArchetypeHashesMap.at(entity);
    return m_archetypesMap.at(archetypeHash).get_archetype();
}

void ecs::ArchetypesDatabase::AddComponent(entity_id entity, const name& componentName)
{
    const archetype& currentArchetype = GetArchetype(entity);
    const component_id componentID = m_componentsRegistry->GetComponentID(componentName);
    if (currentArchetype.has_component(componentID))
    {
        return;
    }

    archetype newArchetype = currentArchetype; // @todo possibly unnecessary copy constructor here
    newArchetype.add_component(componentID);

    MoveEntity(entity, newArchetype);
}

void ecs::ArchetypesDatabase::AddComponent(entity_id entity, const component_id componentID)
{
    ecs::name componentName; 
    ecs::component_data componentData;
    if (m_componentsRegistry->TryGetComponentData(componentID, componentName, componentData))
    {
        AddComponent(entity, componentName);
    }
}

void ecs::ArchetypesDatabase::RemoveComponent(entity_id entity, const name& componentName)
{
    const archetype& currentArchetype = GetArchetype(entity);
    const component_id componentID = m_componentsRegistry->GetComponentID(componentName);
    if (!currentArchetype.has_component(componentID))
    {
        return;
    }

    archetype newArchetype = currentArchetype;
    newArchetype.remove_component(componentID);

    MoveEntity(entity, newArchetype);
}

void ecs::ArchetypesDatabase::RemoveComponent(entity_id entity, const component_id componentID)
{
    ecs::name componentName;
    ecs::component_data componentData;
    if (m_componentsRegistry->TryGetComponentData(componentID, componentName, componentData))
    {
        RemoveComponent(entity, componentName);
    }
}

void ecs::ArchetypesDatabase::MoveEntity(entity_id entity, const archetype& targetArchetype)
{
    const size_t currentArchetypeHash = m_entitiesArchetypeHashesMap.at(entity);
    archetype_set& currentSet = m_archetypesMap.at(currentArchetypeHash);
    const size_t currentIndex = currentSet.get_entity_index(entity);
    
    const size_t targetArchetypeHash = CalculateArchetypeHash(targetArchetype);
    auto optionalTargetSet = m_archetypesMap.find(targetArchetypeHash);
    if (optionalTargetSet == m_archetypesMap.end())
    {
        // The target archetype still doesn't exist, create it
        m_archetypesMap.emplace(targetArchetypeHash, archetype_set(targetArchetype, m_componentsRegistry.get()));
    }

    // allocate memory for storing components of the entity in the new archetype.
    archetype_set& targetSet = m_archetypesMap.at(targetArchetypeHash);
    targetSet.add_entity(entity);
    const size_t targetIndex = targetSet.get_entity_index(entity);

    for (auto componentIt = currentSet.get_archetype().begin(); componentIt != currentSet.get_archetype().end(); ++componentIt)
    {
        const component_id componentID = *componentIt;
        if (!targetSet.get_archetype().has_component(componentID))
        {
            continue;
        }
        
        void* componentPtr = currentSet.get_component_at_index(componentID, currentIndex);
        void* targetComponentPtr = targetSet.get_component_at_index(componentID, targetIndex);
        component_data componentData;
        name componentName;
        if (!m_componentsRegistry->TryGetComponentData(componentID, componentName, componentData))
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
    m_entitiesArchetypeHashesMap[entity] = targetArchetypeHash;

    // remove old set if empty
    if (currentSet.get_num_entities() == 0)
    {
        RemoveArchetypeSet(currentSet.get_archetype());
    }
}

void ecs::ArchetypesDatabase::RemoveEntity(entity_id entity)
{
    auto optionalArchetypeHash = m_entitiesArchetypeHashesMap.find(entity);
    if (optionalArchetypeHash != m_entitiesArchetypeHashesMap.end())
    {
        auto optionalArchetypeSet = m_archetypesMap.find(optionalArchetypeHash->second);
        if (optionalArchetypeSet != m_archetypesMap.end())
        {
            optionalArchetypeSet->second.remove_entity(entity);
        }

        m_entitiesArchetypeHashesMap.erase(entity);
    }
}

void ecs::ArchetypesDatabase::Reset()
{
    m_archetypesMap.clear();
    m_entitiesArchetypeHashesMap.clear();
}

void ecs::ArchetypesDatabase::RemoveArchetypeSet(const ecs::archetype& archetype)
{
    m_archetypesMap.erase(CalculateArchetypeHash(archetype));
}