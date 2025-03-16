#pragma once 

#include <unordered_map>
#include <typeinfo>
#include "Types.h"
#include "IDGenerator.h"
#include "ComponentData.h"

namespace ecs
{
    class ComponentsDatabase
    {
    public:
        template<typename ComponentType>
        static component_id GetComponentID()
        {
            const type_hash_t componentHash = GetTypeHash(ComponentType);
            auto optionalComponentData = s_componentsClassMap.find(componentHash);
            if (optionalComponentData == s_componentsClassMap.end())
            {
                return AddComponentData(componentHash, sizeof(ComponentType), 8);
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        static component_id GetComponentID(const type_hash_t componentHash)
        {
            auto optionalComponentData = s_componentsClassMap.find(componentHash);
            if (optionalComponentData == s_componentsClassMap.end())
            {
                throw std::invalid_argument("Component not found in the database. Call RegisterComponent() first.");
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        static bool TryGetComponentData(const type_hash_t componentHash, component_data& outComponentData);

        template<typename ComponentType>
        static void RegisterComponent(const size_t initialCapacity = 8)
        {
            AddComponentData(GetTypeHash(ComponentType), sizeof(ComponentType), initialCapacity);
        }

    private:
        static ecs::component_id AddComponentData(const type_hash_t componentHash, const size_t dataSize, const size_t initialCapacity = 8);

        static IDGenerator<component_id> s_componentIDGenerator;

        static std::unordered_map<size_t, component_data> s_componentsClassMap;
    };
}