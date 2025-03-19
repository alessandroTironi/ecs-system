#pragma once 

#include <set>
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <iostream>
#include "Types.h"
#include "ComponentsRegistry.h"
#include "ComponentData.h"
#include "PackedComponentArray.h"
#include "Entity.h"

namespace ecs 
{
    struct archetype 
    {
    public:
        archetype();
        archetype(std::initializer_list<component_id> signature);
        archetype(std::initializer_list<component_data> componentsData);
        archetype(const std::set<component_id>&& signature);
        static archetype make(std::initializer_list<component_id> components);

        template<typename FirstComponent, typename... OtherComponents>
        static archetype make(ComponentsRegistry* componentsRegistry)
        {
            std::initializer_list<component_id> signature = 
            { 
                componentsRegistry->GetComponentID<FirstComponent>(), 
                componentsRegistry->GetComponentID<OtherComponents>()... 
            };

            return make(signature);
        }

        inline bool is_null() const { return m_componentIDs.empty(); }

        inline bool has_component(const component_id componentID) const
        {
            return m_componentIDs.contains(componentID);
        }   

        template<typename ComponentType>
        inline bool has_component() const 
        {
            return m_componentIDs.contains(ComponentsRegistry::GetComponentID<ComponentType>());
        }

        inline size_t get_num_components() const { return m_componentIDs.size(); }

        inline void add_component(const component_id componentID)
        {
            m_componentIDs.insert(componentID);
        }

        template<typename ComponentType>
        inline void add_component()
        {
            m_componentIDs.insert(ComponentsRegistry::GetComponentID<ComponentType>());
        }

        inline void remove_component(const component_id componentID)
        {
            m_componentIDs.erase(componentID);
        }

        template<typename ComponentType>
        inline void remove_component()
        {
            m_componentIDs.erase(ComponentsRegistry::GetComponentID<ComponentType>());
        }

        inline auto begin() const { return m_componentIDs.begin(); }
        inline auto end() const { return m_componentIDs.end(); }

        inline bool operator==(const archetype& other) const { return m_componentIDs == other.m_componentIDs; }

    private:
        std::set<component_id> m_componentIDs;
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
            seed ^= std::hash<ecs::component_id>{}(*componentIt) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        return seed;
    }

    static size_t CalculateArchetypeHash(std::initializer_list<component_data> componentsData)
    {
        size_t seed = componentsData.size();
        for (auto componentIt = componentsData.begin(); componentIt != componentsData.end(); ++componentIt)
        {
            seed ^= std::hash<ecs::component_id>{}(componentIt->serial()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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