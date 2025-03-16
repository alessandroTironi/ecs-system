#pragma once

#include "Types.h"

namespace ecs
{
    typedef unsigned short component_id;

    struct component_data
    {
        component_data() = default;
        component_data(const type_hash_t& hash, const size_t& dataSize, const component_id serial, const size_t initialCapacity = 8)
            : m_hash(hash), m_dataSize(dataSize), m_serial(serial), m_initialCapacity(initialCapacity)
        {}

        inline type_hash_t hash() const { return m_hash; }
        inline size_t data_size() const { return m_dataSize; }
        inline size_t initial_capacity() const { return m_initialCapacity; }
        inline component_id serial() const { return m_serial; }

    private:
        type_hash_t m_hash;
        size_t m_dataSize;
        size_t m_initialCapacity;
        component_id m_serial;
    };
}