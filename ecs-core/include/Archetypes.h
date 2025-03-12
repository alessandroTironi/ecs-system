#pragma once 

#include <set>
#include <initializer_list>
#include <stdexcept>
#include "Types.h"
#include "ComponentsDatabase.h"

namespace ecs 
{
    struct archetype 
    {
    public:
        archetype();
        archetype(std::initializer_list<component_id> components);
        archetype(const std::set<component_id>&& components);

        static archetype make(std::initializer_list<component_id> components);

        template<typename FirstComponent, typename... OtherComponents>
        static archetype make()
        {
            std::initializer_list<component_id> components = 
            { 
                ComponentsDatabase::GetComponentID(GetTypeHash(FirstComponent)), 
                ComponentsDatabase::GetComponentID(GetTypeHash(OtherComponents))... 
            };

            return make(components);
        }

        inline bool is_null() const { return m_components.empty(); }

        inline size_t get_num_components() const { return m_components.size(); }

        inline auto begin() const { return m_components.begin(); }
        inline auto end() const { return m_components.end(); }

    private:
        std::set<component_id> m_components;

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