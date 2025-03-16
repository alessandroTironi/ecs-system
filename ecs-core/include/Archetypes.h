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
                ComponentsDatabase::GetComponentID<FirstComponent>(), 
                ComponentsDatabase::GetComponentID<OtherComponents>()... 
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

namespace ecs
{
    template<typename FirstComponent, typename... OtherComponents>
    static size_t CalculateArchetypeHash()
    {
        return std::hash<ecs::archetype>{}(archetype::make<FirstComponent, OtherComponents...>());
    }

    /*
    static size_t CalculateArchetypeHash(std::initializer_list<type_hash_t> componentTypes)
    {
        size_t seed = componentTypes.size();
        for (auto componentIt = componentTypes.begin(); componentIt != componentTypes.end(); ++componentIt)
        {
            const component_id componentSerial = ComponentsDatabase::GetComponentID(*componentIt);
            seed ^= std::hash<ecs::component_id>{}(componentSerial) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
        */

    static size_t CalculateArchetypeHash(std::initializer_list<component_id> componentIDs)
    {
        size_t seed = componentIDs.size();
        for (auto componentIt = componentIDs.begin(); componentIt != componentIDs.end(); ++componentIt)
        {
            seed ^= std::hash<ecs::component_id>{}(*componentIt) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }

    static size_t CalculateArchetypeHash(const std::set<component_id> componentIDs)
    {
        size_t seed = componentIDs.size();
        for (auto componentIt = componentIDs.begin(); componentIt != componentIDs.end(); ++componentIt)
        {
            seed ^= std::hash<ecs::component_id>{}(*componentIt) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }

    static size_t CalculateArchetypeHash(const archetype& archetype)
    {
        size_t seed = archetype.get_num_components();
        for (auto componentIt = archetype.begin(); componentIt != archetype.end(); ++componentIt)
        {
            seed ^= std::hash<ecs::component_id>{}(*componentIt) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }
}

namespace std
{
    template<>
    struct hash<ecs::archetype>
    {
        size_t operator()(const ecs::archetype& archetype) const
        {
            return ecs::CalculateArchetypeHash(archetype);
        }
    };
}

namespace ecs
{
    struct packed_component_array_t
    {
    public:
        packed_component_array_t();
        packed_component_array_t(const type_hash_t& hash, const size_t& sizeOfInstance,
            const component_id serial, const size_t initialSize = 4);
        packed_component_array_t(const packed_component_array_t& other);
        packed_component_array_t(packed_component_array_t&& other) noexcept;
        ~packed_component_array_t();

        packed_component_array_t& operator=(packed_component_array_t&& other)
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

        packed_component_array_t& operator=(const packed_component_array_t& other)
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

    template<typename ComponentType>
    struct packed_component_array : public packed_component_array_t
    {
    public:
        packed_component_array() 
            : packed_component_array_t(GetTypeHash(ComponentType), sizeof(ComponentType), ComponentsDatabase::GetComponentID<ComponentType>())
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

    class ArchetypesDatabase
    {
    public:
        static void AddEntity(entity_id entity, std::initializer_list<component_data> componentTypes);

        template<typename... Components>
        static void AddEntity(entity_id entity)
        {
            AddEntity(entity, { component_data(GetTypeHash(Components), sizeof(Components), 
                ComponentsDatabase::GetComponentID<Components>(), 8)...});
        }

        static size_t GetNumArchetypes() { return s_archetypesMap.size(); }
        static void Reset();

    private:
        struct archetype_set
        {
        public:
            archetype_set(const archetype& archetype);
        private:
            archetype m_archetype;
            std::unordered_map<component_id, std::shared_ptr<packed_component_array_t>> m_componentArraysMap;
            std::unordered_map<entity_id, size_t> m_entityToIndexMap;
        };

        static std::unordered_map<size_t, archetype_set> s_archetypesMap;
    };
}