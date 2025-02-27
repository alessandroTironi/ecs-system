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
        entity_t() = delete;
        entity_t(entity_id uniqueID);

        inline entity_id getUniqueID() const { return m_uniqueID; }

        bool operator==(const entity_t& other)  const { return m_uniqueID == other.getUniqueID(); }

    private:
        entity_id m_uniqueID;
        std::bitset<MAX_COMPONENTS> m_componentsSignature;
    };
}