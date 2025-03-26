#pragma once

#include <iostream>
#include <memory>
#include "ComponentData.h"
#include "ComponentsRegistry.h"

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

        /**
         * @brief Adds a component at the end of the array. 
         * 
         * @return Pointer to the added component.
         */
        void* add_component();
        
        /**
         * @brief Returns a pointer to the component at the given index. 
         * 
         * @param index The index of the component to return.
         * @return Pointer to the component at the given index.
         */
        void* get_component(const size_t index) const;

        /**
         * @brief Returns a pointer to the last component in the array. 
         * 
         * @return Pointer to the last component in the array.
         */
        void* last() const { return get_component(m_size - 1); }

        /**
         * @brief Deletes the component at the given index. 
         * 
         * @param index The index of the component to delete.
         */
        void delete_at(const size_t index);

        /**
         * @brief Copies the component at the given index to the destination array. 
         * 
         * @param index The index of the component to copy.
         * @param destination The destination array to copy the component to.
         */
        void copy_to(size_t index, packed_component_array_t& destination);

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
        packed_component_array(ComponentsRegistry* componentsRegistry) 
            : packed_component_array_t(componentsRegistry->GetOrAddComponentData<ComponentType>())
        {}

        /**
         * @brief Adds a component at the end of the array. 
         * 
         * @return Reference to the added component.
         */
        ComponentType& add_component()
        {
            return *static_cast<ComponentType*>(packed_component_array_t::add_component());
        }

        /**
         * @brief Adds a component at the end of the array, by directly constructing it in place. 
         * 
         * @return Reference to the added component.
         */
        template<typename... Args>
        ComponentType& emplace_component(Args&&... args)
        {
            ComponentType* component = static_cast<ComponentType*>(packed_component_array_t::add_component());
            return *new (component) ComponentType(std::forward<Args>(args)...);
        }

        /**
         * @brief Returns a reference to the component at the given index. 
         * 
         * @param index The index of the component to return.
         * @return Reference to the component at the given index.
         */
        ComponentType& get_component(const size_t index) const
        {
            return *static_cast<ComponentType*>(packed_component_array_t::get_component(index));
        }
    };
}