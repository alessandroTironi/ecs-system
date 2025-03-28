#pragma once 

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
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
            const type_key componentName = typeid(ComponentType);
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

        component_id GetComponentID(const type_key& componentType)
        {
            auto optionalComponentData = m_componentsClassMap.find(componentType);
            if (optionalComponentData == m_componentsClassMap.end())
            {
                throw std::invalid_argument("Component not found in the database. Call RegisterComponent() first.");
            }
            else
            {
                return optionalComponentData->second.serial();
            }
        }

        bool TryGetComponentData(const type_key& componentType, component_data& outComponentData);
        bool TryGetComponentData(const component_id componentID, type_key& outComponentIndex, component_data& outComponentData);

        template<typename ComponentType>
        component_data GetOrAddComponentData()
        {
            const component_id componentID = GetComponentID<ComponentType>(); 
            component_data componentData;
            type_key componentType;
            TryGetComponentData(componentID, componentType, componentData);
            return componentData;   
        }

        template<typename ComponentType>
        void RegisterComponent(const size_t initialCapacity = 8)
        {
            AddComponentData(typeid(ComponentType), sizeof(ComponentType), initialCapacity);
        }

        void Reset()
        {
            m_componentIDGenerator.Reset();
            m_componentsClassMap.clear();
            m_componentTypes.clear();
        }

    private:
        component_id AddComponentData(const type_key& componentType, const size_t dataSize, const size_t initialCapacity = 8);

        IDGenerator<component_id> m_componentIDGenerator;
        std::unordered_map<type_key, component_data> m_componentsClassMap;
        std::vector<type_key> m_componentTypes;
    };
}