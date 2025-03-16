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

//--------------------------------------------------------------
// packed_component_array
//--------------------------------------------------------------

ecs::packed_component_array_t::packed_component_array_t() : m_data(nullptr, [](void*){}),
    m_size{0}, m_hash{0}, m_serial{0}, m_instanceSize{0}, m_capacity{0}
{

}

ecs::packed_component_array_t::packed_component_array_t(const type_hash_t& hash, const size_t& sizeOfInstance,
    const component_id serial, const size_t initialSize) : m_data(nullptr, [](void*){})
{
    m_hash = hash;
    m_serial = serial;
    m_size = 0;
    m_instanceSize = sizeOfInstance;
    m_capacity = initialSize;

    // allocate data
    void* memory = ::operator new[](m_instanceSize * m_capacity);
    
    // Custom deleter that properly destroys all elements and deallocates memory
    auto deleter = [](void* ptr) 
    {
        ::operator delete[](ptr);
    };
    
    m_data = std::unique_ptr<void, void(*)(void*)>(memory, deleter);
}

ecs::packed_component_array_t::packed_component_array_t(const ecs::packed_component_array_t& other)
    : m_data(nullptr, [](void*){})
{
    std::cout << "Calling copy constructor" << std::endl;

    m_hash = other.hash();
    m_serial = other.component_serial();
    m_size = other.size();
    m_instanceSize = other.component_size();
    m_capacity = other.capacity();
    std::memcpy(other.m_data.get(), m_data.get(), m_instanceSize * m_capacity);
}

ecs::packed_component_array_t::packed_component_array_t(ecs::packed_component_array_t&& other) noexcept
: m_data(nullptr, [](void*){})
{
    std::cout << "Calling move constructor" << std::endl;

    std::swap(m_data, other.m_data);
    std::swap(m_size, other.m_size);
    std::swap(m_hash, other.m_hash);
    std::swap(m_serial, other.m_serial);
    std::swap(m_instanceSize, other.m_instanceSize);
    std::swap(m_capacity, other.m_capacity);
}

ecs::packed_component_array_t::~packed_component_array_t()
{
    m_data.release();
}

void* ecs::packed_component_array_t::add_component()
{
    if (m_size == m_capacity)
    {
        // reallocate memory
        m_capacity *= 2;
        void* newMemory = ::operator new[](m_instanceSize * m_capacity);
        std::memcpy(m_data.get(), newMemory, m_instanceSize * m_size);
        m_data.reset(newMemory);
    }

    // address of the next free slot of the array
    void* address = static_cast<void*>(static_cast<char*>(m_data.get()) + m_instanceSize * m_size);
    m_size += 1;
    return address;
}

void* ecs::packed_component_array_t::get_component(const size_t index) const
{
    if (index >= m_size)
    {
        throw std::out_of_range("Index out of bounds");
    }

    return static_cast<void*>(static_cast<char*>(m_data.get()) + m_instanceSize * index);
}

void ecs::packed_component_array_t::delete_at(const size_t index)
{
    if (index >= m_size)
    {
        throw std::out_of_range("Index out of bounds");
    }

    // move the last element to the deleted element 
    void* last = get_component(m_size - 1);
    void* toDelete = get_component(index);
    std::memcpy(last, toDelete, m_instanceSize);
    m_size -= 1;
}

//--------------------------------------------------------------
// ARCHETYPE HASHING
//--------------------------------------------------------------

size_t ecs::CalculateArchetypeHash(std::initializer_list<ecs::component_data> componentsData)
{
    size_t seed = componentsData.size();
    for (auto componentIt = componentsData.begin(); componentIt != componentsData.end(); ++componentIt)
    {
        seed ^= std::hash<ecs::component_id>{}(ComponentsDatabase::GetComponentID((*componentIt).hash())) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

//--------------------------------------------------------------
// ARCHETYPE DATABASE
//--------------------------------------------------------------

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