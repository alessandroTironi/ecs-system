#pragma once 

#include <set>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "Types.h"
#include "ComponentsDatabase.h"
#include "ComponentData.h"
#include "PackedComponentArray.h"
#include "Entity.h"

namespace ecs 
{
    struct archetype 
    {
    public:
        archetype();
        archetype(std::initializer_list<type_hash_t> signature);
        archetype(std::initializer_list<component_data> componentsData);
        archetype(const std::set<type_hash_t>&& signature);

        static archetype make(std::initializer_list<type_hash_t> components);

        template<typename FirstComponent, typename... OtherComponents>
        static archetype make()
        {
            std::initializer_list<type_hash_t> signature = 
            { 
                GetTypeHash(FirstComponent), 
                GetTypeHash(OtherComponents)... 
            };

            return make(signature);
        }

        inline bool is_null() const { return m_componentTypes.empty(); }

        inline bool has_component(const type_hash_t componentType) const { return m_componentTypes.contains(componentType); }
        inline size_t get_num_components() const { return m_componentTypes.size(); }

        inline void add_component(const type_hash_t componentType) { m_componentTypes.insert(componentType); }

        inline auto begin() const { return m_componentTypes.begin(); }
        inline auto end() const { return m_componentTypes.end(); }

    private:
        std::set<type_hash_t> m_componentTypes;
    };
}

namespace ecs
{
    template<typename FirstComponent, typename... OtherComponents>
    static size_t CalculateArchetypeHash()
    {
        return std::hash<ecs::archetype>{}(archetype::make<FirstComponent, OtherComponents...>());
    }

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
            seed ^= std::hash<ecs::component_id>{}(ComponentsDatabase::GetComponentID(*componentIt)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }

    static size_t CalculateArchetypeHash(std::initializer_list<component_data> componentsData)
    {
        size_t seed = componentsData.size();
        for (auto componentIt = componentsData.begin(); componentIt != componentsData.end(); ++componentIt)
        {
            seed ^= std::hash<ecs::component_id>{}(ComponentsDatabase::GetComponentID((*componentIt).hash())) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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