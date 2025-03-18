#pragma once 

#include <bitset>
#include "Types.h"
#include "ComponentData.h"

#define MAX_COMPONENTS 1000

namespace ecs
{
    class Instance;

    typedef size_t entity_id;

    struct entity_t
    {
    public:
        entity_t();

        void add_component(const component_id componentID);
        void remove_component(const component_id componentID);
        bool has_component(const component_id componentID) const;
    private:
        std::bitset<MAX_COMPONENTS> m_componentsSignature;
    };
}