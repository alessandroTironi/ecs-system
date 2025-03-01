#pragma once 

#include <bitset>

#define MAX_COMPONENTS 1000

namespace ecs
{
    class Instance;

    typedef size_t entity_id;

    struct entity_t
    {
    public:
        entity_t();

    private:
        std::bitset<MAX_COMPONENTS> m_componentsSignature;
    };
}