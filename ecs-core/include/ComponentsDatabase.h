#pragma once 

#include <unordered_map>
#include <typeinfo>
#include <vector>
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
            const std::string componentName = typeid(ComponentType).name();
            auto optionalComponentData = s_componentsClassMap.find(componentName);
            if (optionalComponentData == s_componentsClassMap.end())
            {
                return AddComponentData(componentName, sizeof(ComponentType), 8);
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        static component_id GetComponentID(const name& componentName)
        {
            auto optionalComponentData = s_componentsClassMap.find(componentName);
            if (optionalComponentData == s_componentsClassMap.end())
            {
                throw std::invalid_argument("Component not found in the database. Call RegisterComponent() first.");
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        static bool TryGetComponentData(const name& componentName, component_data& outComponentData);
        static bool TryGetComponentData(const component_id componentID, name& outComponentName, component_data& outComponentData);

        template<typename ComponentType>
        static component_data GetOrAddComponentData()
        {
            const component_id componentID = GetComponentID<ComponentType>(); 
            component_data componentData;
            name componentName;
            TryGetComponentData(componentID, componentName, componentData);
            return componentData;   
        }

        template<typename ComponentType>
        static void RegisterComponent(const size_t initialCapacity = 8)
        {
            AddComponentData(typeid(ComponentType).name(), sizeof(ComponentType), initialCapacity);
        }

        static void Reset()
        {
            s_componentIDGenerator.Reset();
            s_componentsClassMap.clear();
            s_componentNames.clear();
        }

    private:
        static ecs::component_id AddComponentData(const name& componentName, const size_t dataSize, const size_t initialCapacity = 8);

        static IDGenerator<component_id> s_componentIDGenerator;

        static std::unordered_map<name, component_data> s_componentsClassMap;
        static std::vector<name> s_componentNames;
    };
}