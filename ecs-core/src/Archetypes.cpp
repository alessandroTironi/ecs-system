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

    m_signature = std::move(components);
}

ecs::archetype::archetype(const std::set<ecs::component_id>&& components)
{
    if (components.size() == 0)
    {
        throw std::invalid_argument("Attempted to create an archetype with no components");
    }

    m_signature = std::move(components);
}

ecs::archetype ecs::archetype::make(std::initializer_list<component_id> components)
{
    return ecs::archetype(std::move(components));
}

//--------------------------------------------------------------
// packed_component_array
//--------------------------------------------------------------

ecs::packed_component_array::packed_component_array() : m_data(nullptr, [](void*){}),
    m_count{0}, m_hash{0}, m_serial{0}, m_instanceSize{0}, m_reservedSize{0}
{

}

ecs::packed_component_array::packed_component_array(const type_hash_t& hash, const size_t& sizeOfInstance,
    const size_t initialSize) : m_data(nullptr, [](void*){})
{
    m_hash = hash;
    m_serial = ComponentsDatabase::GetComponentID(hash);
    m_count = 0;
    m_instanceSize = sizeOfInstance;
    m_reservedSize = initialSize;

    // allocate data
    void* memory = ::operator new[](m_instanceSize * m_reservedSize);
    
    // Custom deleter that properly destroys all elements and deallocates memory
    auto deleter = [](void* ptr) 
    {
        ::operator delete[](ptr);
    };
    
    m_data = std::unique_ptr<void, void(*)(void*)>(memory, deleter);
}

ecs::packed_component_array::packed_component_array(const ecs::packed_component_array& other)
    : m_data(nullptr, [](void*){})
{
    std::cout << "Calling copy constructor" << std::endl;

    m_hash = other.hash();
    m_serial = other.component_serial();
    m_count = other.count();
    m_instanceSize = other.component_size();
    m_reservedSize = other.reserved_size();
    std::memcpy(other.m_data.get(), m_data.get(), m_instanceSize * m_reservedSize);
}

ecs::packed_component_array::packed_component_array(ecs::packed_component_array&& other) noexcept
: m_data(nullptr, [](void*){})
{
    std::cout << "Calling move constructor" << std::endl;

    std::swap(m_data, other.m_data);
    std::swap(m_count, other.m_count);
    std::swap(m_hash, other.m_hash);
    std::swap(m_serial, other.m_serial);
    std::swap(m_instanceSize, other.m_instanceSize);
    std::swap(m_reservedSize, other.m_reservedSize);
}

ecs::packed_component_array::~packed_component_array()
{
    m_data.release();
}

//--------------------------------------------------------------
// ARCHETYPE DATABASE
//--------------------------------------------------------------

std::unordered_map<size_t, ecs::ArchetypesDatabase::archetype_set> ecs::ArchetypesDatabase::s_archetypesMap;

ecs::ArchetypesDatabase::archetype_set::archetype_set(const ecs::archetype& archetype)
{
    m_archetype = std::move(archetype);
}

void ecs::ArchetypesDatabase::AddEntity(std::initializer_list<type_hash_t> componentTypes)
{
    throw std::runtime_error("Not implemented");
}

void ecs::ArchetypesDatabase::AddEntity(const std::vector<type_hash_t>& componentTypes)
{
    throw std::runtime_error("Not implemented");
}

void ecs::ArchetypesDatabase::Reset()
{
    s_archetypesMap.clear();
}