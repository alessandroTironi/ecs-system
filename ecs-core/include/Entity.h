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

        void add_component(const type_hash_t componentHash);
        void remove_component(const type_hash_t componentHash);
        bool has_component(const type_hash_t componentHash) const;
    private:
        std::bitset<MAX_COMPONENTS> m_componentsSignature;
    };
}