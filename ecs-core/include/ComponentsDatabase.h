#pragma once 

#include <unordered_map>
#include <typeinfo>
#include "Types.h"
#include "IDGenerator.h"

namespace esc
{
    typedef unsigned short component_id;

    class ComponentDatabase
    {
    public:
        template<typename ComponentType>
        static component_id GetComponentID()
        {
            const type_hash_t componentHash = GetTypeHash(ComponentType);
            return GetComponentID(componentHash);
        }

        static component_id GetComponentID(const type_hash_t componentHash)
        {
            auto optionalID = s_componentsClassMap.find(componentHash);
            if (optionalID == s_componentsClassMap.end())
            {
                const component_id newID = s_componentIDGenerator.GenerateNewUniqueID();
                s_componentsClassMap[componentHash] = newID;
                return newID;
            }
            else
            {
                return optionalID->second;
            }
        }

    private:
        static IDGenerator<component_id> s_componentIDGenerator;

        static std::unordered_map<size_t, component_id> s_componentsClassMap;
    }
}