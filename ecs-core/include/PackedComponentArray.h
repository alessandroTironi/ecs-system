#pragma once

#include <iostream>
#include <memory>
#include "ComponentData.h"
#include "ComponentsDatabase.h"

namespace ecs
{
	struct packed_component_array_t
    {
    public:
        packed_component_array_t();
        packed_component_array_t(const component_data& componentData);
        packed_component_array_t(const packed_component_array_t& other);
        packed_component_array_t(packed_component_array_t&& other) noexcept;
        ~packed_component_array_t();

        packed_component_array_t& operator=(packed_component_array_t&& other)
        {
            std::swap(m_data, other.m_data);
            std::swap(m_size, other.m_size);
            std::swap(m_serial, other.m_serial);
            std::swap(m_instanceSize, other.m_instanceSize);
            std::swap(m_capacity, other.m_capacity);
            return *this;
        }

        packed_component_array_t& operator=(const packed_component_array_t& other)
        {
            m_serial = other.component_serial();
            m_size = other.size();
            m_instanceSize = other.component_size();
            m_capacity = other.capacity();
            std::memcpy(other.m_data.get(), m_data.get(), m_instanceSize * m_capacity);
            return *this;
        }

        inline size_t size() const { return m_size; }
        inline component_id component_serial() const { return m_serial; }
        inline size_t component_size() const { return m_instanceSize; }
        inline size_t capacity() const { return m_capacity; }

        void* add_component();
        void* get_component(const size_t index) const;
        void delete_at(const size_t index);

    private:
        std::unique_ptr<void, void(*)(void*)> m_data;
        size_t m_size;
        component_id m_serial;
        size_t m_instanceSize;
        size_t m_capacity;
    };

    template<typename ComponentType>
    struct packed_component_array : public packed_component_array_t
    {
    public:
        packed_component_array(ComponentsDatabase* componentsRegistry) 
            : packed_component_array_t(componentsRegistry->GetOrAddComponentData<ComponentType>())
        {}

        ComponentType& add_component()
        {
            return *static_cast<ComponentType*>(packed_component_array_t::add_component());
        }

        template<typename... Args>
        ComponentType& emplace_component(Args&&... args)
        {
            ComponentType* component = static_cast<ComponentType*>(packed_component_array_t::add_component());
            return *new (component) ComponentType(std::forward<Args>(args)...);
        }

        ComponentType& get_component(const size_t index) const
        {
            return *static_cast<ComponentType*>(packed_component_array_t::get_component(index));
        }
    };
}