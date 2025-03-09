#pragma once 

#include <unordered_map>
#include <typeinfo>
#include "Types.h"
#include "IDGenerator.h"

namespace ecs
{
    typedef unsigned short component_id;

    class ComponentsDatabase
    {
    public:
        template<typename ComponentType>
        static component_id GetComponentID()
        {
            const type_hash_t componentHash = GetTypeHash(ComponentType);
            return GetComponentID(componentHash);
        }

        static component_id GetComponentID(const type_hash_t componentHash);

    private:
        static IDGenerator<component_id> s_componentIDGenerator;

        static std::unordered_map<size_t, component_id> s_componentsClassMap;
    };
}