#pragma once 

#include <set>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "Types.h"
#include "ComponentsDatabase.h"
#include "ComponentArray.h"

namespace ecs 
{
    struct archetype 
    {
    public:
        archetype();
        archetype(std::initializer_list<component_id> signature);
        archetype(const std::set<component_id>&& signature);

        static archetype make(std::initializer_list<component_id> components);

        template<typename FirstComponent, typename... OtherComponents>
        static archetype make()
        {
            std::initializer_list<component_id> signature = 
            { 
                ComponentsDatabase::GetComponentID(GetTypeHash(FirstComponent)), 
                ComponentsDatabase::GetComponentID(GetTypeHash(OtherComponents))... 
            };

            return make(signature);
        }

        inline bool is_null() const { return m_signature.empty(); }

        inline size_t get_num_components() const { return m_signature.size(); }

        inline auto begin() const { return m_signature.begin(); }
        inline auto end() const { return m_signature.end(); }

    private:
        std::set<component_id> m_signature;
    };
}

namespace std
{
    template<>
    struct hash<ecs::archetype>
    {
        size_t operator()(const ecs::archetype& archetype) const
        {
            size_t seed = archetype.get_num_components();
            for (auto componentIt = archetype.begin(); componentIt != archetype.end(); ++componentIt)
            {
                seed ^= std::hash<ecs::component_id>{}(*componentIt) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            return seed;
        }
    };
}

namespace ecs
{
    template<typename FirstComponent, typename... OtherComponents>
    static size_t CalculateArchetypeHash()
    {
        return std::hash<ecs::archetype>{}(archetype::make<FirstComponent, OtherComponents...>());
    }
}

namespace ecs
{
    struct packed_component_array
    {
    public:
        packed_component_array();
        packed_component_array(const type_hash_t& hash, const size_t& sizeOfInstance,
            const size_t initialSize = 4);
        packed_component_array(const packed_component_array& other);
        packed_component_array(packed_component_array&& other) noexcept;
        ~packed_component_array();

        packed_component_array& operator=(packed_component_array&& other)
        {
            std::cout << "Calling move operator assignment" << std::endl;
            std::swap(m_data, other.m_data);
            std::swap(m_size, other.m_size);
            std::swap(m_hash, other.m_hash);
            std::swap(m_serial, other.m_serial);
            std::swap(m_instanceSize, other.m_instanceSize);
            std::swap(m_capacity, other.m_capacity);
            return *this;
        }

        packed_component_array& operator=(const packed_component_array& other)
        {
            std::cout << "Calling copy operator assignment" << std::endl;
            m_hash = other.hash();
            m_serial = other.component_serial();
            m_size = other.size();
            m_instanceSize = other.component_size();
            m_capacity = other.capacity();
            std::memcpy(other.m_data.get(), m_data.get(), m_instanceSize * m_capacity);
            return *this;
        }

        inline size_t size() const { return m_size; }
        inline type_hash_t hash() const { return m_hash; }
        inline component_id component_serial() const { return m_serial; }
        inline size_t component_size() const { return m_instanceSize; }
        inline size_t capacity() const { return m_capacity; }

        void* add_component();
        void* get_component(const size_t index) const;
        void delete_at(const size_t index);

    private:
        std::unique_ptr<void, void(*)(void*)> m_data;
        size_t m_size;
        type_hash_t m_hash;
        component_id m_serial;
        size_t m_instanceSize;
        size_t m_capacity;
    };

    class ArchetypesDatabase
    {
    public:
        static void AddEntity(std::initializer_list<type_hash_t> componentTypes);
        static void AddEntity(const std::vector<type_hash_t>& componentTypes);

        static void Reset();

    private:
        struct archetype_set
        {
        public:
            archetype_set(const archetype& archetype);
        private:
            archetype m_archetype;
            std::unordered_map<component_id, std::shared_ptr<component_array_base>> m_componentArraysMap;
        };

        static std::unordered_map<size_t, archetype_set> s_archetypesMap;
    };
}