#pragma once

#include "Types.h"

namespace ecs
{
    typedef unsigned short component_id;

    struct IComponent {};

    struct component_data
    {
        component_data() = default;
        component_data(const size_t& dataSize, const component_id serial, const size_t initialCapacity = 8)
            : m_dataSize(dataSize), m_serial(serial), m_initialCapacity(initialCapacity)
        {}

        inline size_t data_size() const { return m_dataSize; }
        inline size_t initial_capacity() const { return m_initialCapacity; }
        inline component_id serial() const { return m_serial; }

    private:
        size_t m_dataSize;
        size_t m_initialCapacity;
        component_id m_serial;
    };
}