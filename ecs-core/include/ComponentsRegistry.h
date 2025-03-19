#pragma once 

#include <unordered_map>
#include <typeinfo>
#include <vector>
#include "Types.h"
#include "IDGenerator.h"
#include "ComponentData.h"

namespace ecs
{
    class ComponentsRegistry
    {
    public:
        ComponentsRegistry() = default;
        ~ComponentsRegistry() = default;

        template<typename ComponentType>
        component_id GetComponentID()
        {
            const name componentName = typeid(ComponentType).name();
            auto optionalComponentData = m_componentsClassMap.find(componentName);
            if (optionalComponentData == m_componentsClassMap.end())
            {
                return AddComponentData(componentName, sizeof(ComponentType), 8);
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        component_id GetComponentID(const name& componentName)
        {
            auto optionalComponentData = m_componentsClassMap.find(componentName);
            if (optionalComponentData == m_componentsClassMap.end())
            {
                throw std::invalid_argument("Component not found in the database. Call RegisterComponent() first.");
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        bool TryGetComponentData(const name& componentName, component_data& outComponentData);
        bool TryGetComponentData(const component_id componentID, name& outComponentName, component_data& outComponentData);

        template<typename ComponentType>
        component_data GetOrAddComponentData()
        {
            const component_id componentID = GetComponentID<ComponentType>(); 
            component_data componentData;
            name componentName;
            TryGetComponentData(componentID, componentName, componentData);
            return componentData;   
        }

        template<typename ComponentType>
        void RegisterComponent(const size_t initialCapacity = 8)
        {
            AddComponentData(typeid(ComponentType).name(), sizeof(ComponentType), initialCapacity);
        }

        void Reset()
        {
            m_componentIDGenerator.Reset();
            m_componentsClassMap.clear();
            m_componentNames.clear();
        }

    private:
        component_id AddComponentData(const name& componentName, const size_t dataSize, const size_t initialCapacity = 8);

        IDGenerator<component_id> m_componentIDGenerator;
        std::unordered_map<name, component_data> m_componentsClassMap;
        std::vector<name> m_componentNames;
    };
}