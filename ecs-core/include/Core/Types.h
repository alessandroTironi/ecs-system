#pragma once 

#include <array>
#include <unordered_map>
#include <vector>
#include <limits>
#include <typeinfo>
#include <typeindex>
#include "CompactString.h"
#include "Containers/memory.h"
#include "Containers/UnorderedMapPoolAllocator.h"

#define MAX_COMPONENTS 2048
#define MAX_ENTITIES 80000

namespace ecs
{
    typedef float real_t;

    typedef size_t entity_id;
    const static entity_id INVALID_ENTITY_ID = std::numeric_limits<entity_id>::max();

    typedef unsigned int archetype_id;

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

    // Definition of memory buckets for the pool memory system.
    struct bucket_entityArchetypePair
    {
        static constexpr size_t s_blockSize = sizeof(std::pair<entity_id, archetype_id>);
        static constexpr size_t s_blockCount = 21000;
    };

    struct bucket_entityArchetypeHashNode
    {
        static constexpr size_t s_blockSize = 
            sizeof(std::__detail::_Hash_node<std::pair<entity_id, archetype_id>, false>);
        static constexpr size_t s_blockCount = 30000;
    };

    template<>
    struct memory_pool::bucket_descriptors<2>
    {
        using type = std::tuple<bucket_entityArchetypePair, bucket_entityArchetypeHashNode>;
    };

    template<typename TKey, typename TValue, size_t MaxPairs = 10000>
    using pm_unordered_map = std::unordered_map<
        TKey,
        TValue,
        std::hash<TKey>,
        std::equal_to<TKey>,
        memory_pool::unordered_map_pool_allocator<std::pair<const TKey, TValue>, TKey, TValue, MaxPairs, MaxPairs>
    >;
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

