#pragma once 

#include <array>
#include <unordered_map>
#include <vector>
#include <limits>
#include <typeinfo>
#include <typeindex>
#include "CompactString.h"

namespace ecs
{
    typedef float real_t;

    typedef size_t entity_id;
    const static entity_id INVALID_ENTITY_ID = std::numeric_limits<entity_id>::max();

    typedef unsigned int archetype_id;

    #define GetTypeHash(obj) typeid(obj).hash_code()

    /**
     * @brief A key for types.
     * 
     * This struct is used to identify types in a type-safe manner.
     * It is a simple wrapper around std::type_index, adding a default constructor that sets the index to typeid(void).
     */
    struct type_key
    {
        type_key() : m_typeIndex(std::type_index(typeid(void))) {}
        type_key(const std::type_index& typeIndex)
            : m_typeIndex(typeIndex) {}
        type_key(const std::type_info& typeInfo)
            : m_typeIndex(std::type_index(typeInfo)) {}

        inline std::type_index type_index() const { return m_typeIndex; }

        bool operator==(const type_key& other) const { return m_typeIndex == other.m_typeIndex; }
        bool operator!=(const type_key& other) const { return m_typeIndex != other.m_typeIndex; }

    private:
        std::type_index m_typeIndex;
    };

    typedef compact_string<40> name;
}

namespace std
{
    template<>
    struct hash<ecs::type_key>
    {
        size_t operator()(const ecs::type_key& key) const 
        {
            return std::hash<std::type_index>{}(key.type_index());
        }
    };
}

