#include "Core/PackedComponentArray.h"

ecs::packed_component_array_t::packed_component_array_t() : m_data(nullptr, [](void*){}),
    m_size{0}, m_serial{0}, m_instanceSize{0}, m_capacity{0}
{

}

ecs::packed_component_array_t::packed_component_array_t(const component_data& componentData) : m_data(nullptr, [](void*){})
{
    m_serial = componentData.serial();
    m_size = 0;
    m_instanceSize = componentData.data_size();
    m_capacity = componentData.initial_capacity();

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
    m_serial = other.component_serial();
    m_size = other.size();
    m_instanceSize = other.component_size();
    m_capacity = other.capacity();
    std::memcpy(other.m_data.get(), m_data.get(), m_instanceSize * m_capacity);
}

ecs::packed_component_array_t::packed_component_array_t(ecs::packed_component_array_t&& other) noexcept
: m_data(nullptr, [](void*){})
{
    std::swap(m_data, other.m_data);
    std::swap(m_size, other.m_size);
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

void ecs::packed_component_array_t::copy_to(size_t index, ecs::packed_component_array_t& destination, size_t destinationIndex)
{
    if (index >= m_size)
    {
        throw std::out_of_range("Source index out of bounds");
    }

    if (destinationIndex >= destination.size())
    {
        throw std::out_of_range("Destination index out of bounds");
    }

    // get pointer to the source component
    void* source = get_component(index);

    // perform copy
    void* destinationPtr = destination.get_component(destinationIndex);
    std::memcpy(destinationPtr, source, m_instanceSize);
}
