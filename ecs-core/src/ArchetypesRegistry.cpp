#include "ArchetypesRegistry.h"
#include "World.h"
#include <algorithm>


ecs::ArchetypesRegistry::archetype_set::archetype_set(const ecs::archetype& archetype, 
    ecs::ComponentsRegistry* componentsRegistry)
{
    m_archetype = archetype;

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

size_t ecs::ArchetypesRegistry::archetype_set::add_entity(entity_id entity)
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

size_t ecs::ArchetypesRegistry::archetype_set::get_entity_index(entity_id entity) const
{
    return m_entityToIndexMap.at(entity);
}

bool ecs::ArchetypesRegistry::archetype_set::try_get_entity_index(entity_id entity, size_t& index) const
{
    auto optionalIndex = m_entityToIndexMap.find(entity);
    if (optionalIndex != m_entityToIndexMap.end())
    {
        index = optionalIndex->second;
        return true;
    }

    return false;
}

void* ecs::ArchetypesRegistry::archetype_set::get_component_at_index(const component_id componentID, const size_t index) const
{
    std::shared_ptr<packed_component_array_t> packedArray = m_componentArraysMap.at(componentID);
    if (packedArray.get() != nullptr)
    {
        return packedArray->get_component(index);
    }

    throw std::runtime_error("Found a null packed_component_array");
}

void* ecs::ArchetypesRegistry::archetype_set::find_component_at_index(const component_id componentID, const size_t index) const
{
    auto optionalArray = m_componentArraysMap.find(componentID);
    if (optionalArray != m_componentArraysMap.end())
    {
        std::shared_ptr<packed_component_array_t> packedArray = optionalArray->second;
        if (packedArray.get() != nullptr)
        {
            return packedArray->get_component(index);
        }
    }

    return nullptr;
}

void ecs::ArchetypesRegistry::archetype_set::remove_entity(ecs::entity_id entity)
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

void ecs::ArchetypesRegistry::archetype_set::copy_entity_to(const entity_id entity, archetype_set& destination)
{
    destination.add_entity(entity);

    for (auto componentIt = m_archetype.begin(); componentIt != m_archetype.end(); ++componentIt)
    {
        if (!destination.get_archetype().has_component(*componentIt))
        {
            continue;
        }

        std::shared_ptr<packed_component_array_t> sourceComponentArray = m_componentArraysMap.at(*componentIt);
        std::shared_ptr<packed_component_array_t> targetComponentArray = destination.m_componentArraysMap.at(*componentIt);

        const size_t entityIndexInSource = get_entity_index(entity);
        sourceComponentArray->copy_to(entityIndexInSource, *targetComponentArray.get());
    }
}

void ecs::ArchetypesRegistry::AddEntity(ecs::entity_id entity, std::initializer_list<ecs::component_data> componentsData)
{
    AddEntity(entity, archetype(componentsData));
}

void ecs::ArchetypesRegistry::AddEntity(entity_id entity, const ecs::archetype& archetype)
{
    // Find the archetype unique ID, or generate it if it doesn't exist yet.
    const archetype_id id = GetOrCreateArchetypeID(archetype);

    // Add the entity to the archetype set.
    archetype_set& archetypeSet = m_archetypeSets[id];
    archetypeSet.add_entity(entity);

    // associate the entity to that archetype hash.
    m_entitiesArchetypeHashesMap[entity] = id;
}

void ecs::ArchetypesRegistry::Reset()
{
    m_archetypeSets.clear();
    m_archetypesIDMap.clear();
    m_entitiesArchetypeHashesMap.clear();
    m_archetypeIDGenerator.Reset();
    m_componentToArchetypeSetMap.clear();
}

void* ecs::ArchetypesRegistry::GetComponent(entity_id entity, const component_id componentID)
{
    const archetype_id archetypeID = m_entitiesArchetypeHashesMap.at(entity);
    const archetype_set& set = m_archetypeSets[archetypeID];
    const size_t entityIndex = set.get_entity_index(entity);
    return set.get_component_at_index(componentID, entityIndex);
}

void* ecs::ArchetypesRegistry::FindComponent(entity_id entity, const component_id componentID)
{
    auto optionalArchetypeID = m_entitiesArchetypeHashesMap.find(entity);
    if (optionalArchetypeID != m_entitiesArchetypeHashesMap.end())
    {
        const archetype_set& set = m_archetypeSets[optionalArchetypeID->second];
        size_t entityIndex = -1;
        if (set.try_get_entity_index(entity, entityIndex))
        {
            return set.find_component_at_index(componentID, entityIndex);
        }
    }

    return nullptr;
}

const ecs::archetype& ecs::ArchetypesRegistry::GetArchetype(entity_id entity) const
{
    const archetype_id archetypeID = m_entitiesArchetypeHashesMap.at(entity);
    return m_archetypeSets[archetypeID].get_archetype();
}

ecs::archetype_id ecs::ArchetypesRegistry::GetArchetypeID(entity_id entity) const
{
    return m_entitiesArchetypeHashesMap.at(entity);
}

void ecs::ArchetypesRegistry::AddComponent(entity_id entity, const name& componentName)
{
    const archetype& currentArchetype = GetArchetype(entity);
    const component_id componentID = m_world->GetComponentsRegistry()->GetComponentID(componentName);
    if (currentArchetype.has_component(componentID))
    {
        return;
    }

    archetype newArchetype = currentArchetype; // @todo possibly unnecessary copy constructor here
    newArchetype.add_component(componentID);

    MoveEntity(entity, newArchetype);
}

void ecs::ArchetypesRegistry::AddComponent(entity_id entity, const component_id componentID)
{
    ecs::name componentName; 
    ecs::component_data componentData;
    if (GetComponentsRegistry()->TryGetComponentData(componentID, componentName, componentData))
    {
        AddComponent(entity, componentName);
    }
}

void ecs::ArchetypesRegistry::RemoveComponent(entity_id entity, const name& componentName)
{
    archetype_id archetypeID;
    const archetype& currentArchetype = GetArchetype(entity);
    const component_id componentID = GetComponentsRegistry()->GetComponentID(componentName);
    if (!currentArchetype.has_component(componentID))
    {
        return;
    }

    archetype newArchetype = currentArchetype;
    newArchetype.remove_component(componentID);

    MoveEntity(entity, newArchetype);
}

void ecs::ArchetypesRegistry::RemoveComponent(entity_id entity, const component_id componentID)
{
    ecs::name componentName;
    ecs::component_data componentData;
    if (GetComponentsRegistry()->TryGetComponentData(componentID, componentName, componentData))
    {
        RemoveComponent(entity, componentName);
    }
}

void ecs::ArchetypesRegistry::MoveEntity(entity_id entity, const archetype& targetArchetype)
{
    const archetype_id targetArchetypeID = GetOrCreateArchetypeID(targetArchetype);

    const archetype_id currentArchetypeID = m_entitiesArchetypeHashesMap.at(entity);
    archetype_set& currentSet = m_archetypeSets[currentArchetypeID];

    // allocate memory for storing components of the entity in the new archetype.
    archetype_set& targetSet = m_archetypeSets[targetArchetypeID];

    // remove copy all components to new set and remove entity from current one.
    currentSet.copy_entity_to(entity, targetSet);
    currentSet.remove_entity(entity);

    // update entities to archetypes map
    m_entitiesArchetypeHashesMap[entity] = targetArchetypeID;
}

void ecs::ArchetypesRegistry::RemoveEntity(entity_id entity)
{
    auto optionalArchetypeID = m_entitiesArchetypeHashesMap.find(entity);
    if (optionalArchetypeID != m_entitiesArchetypeHashesMap.end())
    {
        archetype_set& archetypeSet = m_archetypeSets[optionalArchetypeID->second];
        archetypeSet.remove_entity(entity);

        m_entitiesArchetypeHashesMap.erase(entity);
    }
}

ecs::archetype_id ecs::ArchetypesRegistry::GetOrCreateArchetypeID(const archetype& archetype)
{
    auto optionalArchetypeID = m_archetypesIDMap.find(archetype);
    if (optionalArchetypeID == m_archetypesIDMap.end())
    {
        const archetype_id id = m_archetypeIDGenerator.GenerateNewUniqueID();
        m_archetypesIDMap[archetype] = id; 
        m_archetypeSets.emplace_back(archetype, GetComponentsRegistry());

        // update the component to archetype map for consistent querying
        for (const component_id componentID : archetype)
        {
            m_componentToArchetypeSetMap[componentID].insert(id);
        }

        return id;
    }
    else
    {
        return optionalArchetypeID->second;
    }
}

ecs::ArchetypesRegistry::archetype_set& ecs::ArchetypesRegistry::GetOrCreateArchetypeSet(const archetype& archetype)
{
    const archetype_id id = GetOrCreateArchetypeID(archetype);
    return m_archetypeSets[id];
}

void ecs::ArchetypesRegistry::QueryEntities(std::initializer_list<component_id> components, std::vector<entity_id>& entities)
{
    std::set<archetype_id> matchingArchetypes;
    QueryArchetypes(components, matchingArchetypes);
    
    // get all the entities from the matching archetypes 
    entities.clear();
    for (const archetype_id archetypeID : matchingArchetypes)
    {
        const archetype_set& archetypeSet = m_archetypeSets[archetypeID];
        entities.reserve(entities.capacity() + archetypeSet.get_num_entities());
        
        for (const std::pair<entity_id, size_t>& entityPair : archetypeSet.entity_map())
        {
            entities.push_back(entityPair.first);
        }
    }
}

void ecs::ArchetypesRegistry::QueryArchetypes(std::initializer_list<component_id> components, 
    std::set<archetype_id>& matchingArchetypes)
{
    matchingArchetypes.clear();

    if (components.size() == 0)
    {
        for (size_t i = 0; i < m_archetypeSets.size(); ++i)
        {
            matchingArchetypes.insert(i);
        }
    }
    else
    {
        auto componentIt = components.begin();
        matchingArchetypes = m_componentToArchetypeSetMap[*componentIt];

        // reduce the starting set by intersecting with all the other component's sets 
        {
            std::set<archetype_id> temp;

            for (++componentIt; componentIt != components.end(); ++componentIt)
            {
                temp.clear();
                const std::set<archetype_id>& currentSet = m_componentToArchetypeSetMap[*componentIt];
                std::set_intersection(matchingArchetypes.begin(), matchingArchetypes.end(),
                                    currentSet.begin(), currentSet.end(),
                                    std::inserter(temp, temp.begin()));
                matchingArchetypes = std::move(temp);
            }
        }
    }
}

ecs::ComponentsRegistry* ecs::ArchetypesRegistry::GetComponentsRegistry() const
{
    return m_world->GetComponentsRegistry().get();
}

ecs::World* ecs::ArchetypesRegistry::GetWorld() const
{   
    return m_world.get();
}
