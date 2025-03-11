#pragma once 

#include <set>
#include <initializer_list>
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

    private:
        std::set<component_id> m_components;

    };
}