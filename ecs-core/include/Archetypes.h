#pragma once 

#include <vector>
#include "Types.h"
#include "ComponentsDatabase.h"

namespace ecs 
{
    struct archetype 
    {
    public:
        template<typename... ComponentTypes>
        static archetype make()
        {
            return archetype();
        }

        static archetype make(const std::vector<component_id>& components);

    private:
        std::vector<component_id> m_components;

    };
}