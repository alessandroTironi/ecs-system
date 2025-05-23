#pragma once 

#include <memory>
#include <limits>
#include "Types.h"
#include "ComponentData.h"
#include "ComponentsRegistry.h"

namespace ecs
{
    class World;
    class ArchetypesRegistry;
    class BatchComponentActionProcessor;

    class EntityHandle
    {
        friend class QueryEntityHandle;
    public:
        EntityHandle();
        EntityHandle(std::weak_ptr<World> world, entity_id id, archetype_id archetypeID, 
            std::shared_ptr<BatchComponentActionProcessor> batchComponentActionProcessor = nullptr);

        template<typename ComponentType>
        void AddComponent()
        {
            if (ComponentsRegistry* componentsRegistry = GetComponentsRegistry())
            {
                const component_id componentID = componentsRegistry->GetComponentID<ComponentType>();
                AddComponent(componentID);
            }
        }

        template<typename ComponentType>
        void DeferredAddComponent()
        {
            if (ComponentsRegistry* componentsRegistry = GetComponentsRegistry())
            {
                const component_id componentID = componentsRegistry->GetComponentID<ComponentType>();
                DeferredAddComponent(componentID);
            }
        }

        template<typename ComponentType>
        ComponentType& GetComponent() const
        {
            if (ComponentsRegistry* componentsRegistry = GetComponentsRegistry())
            {
                const component_id componentID = componentsRegistry->GetComponentID<ComponentType>();
                return *static_cast<ComponentType*>(GetComponent(componentID));
            }

            throw std::runtime_error("Components registry not found");
        }

        template<typename ComponentType>
        ComponentType* FindComponent() const
        {
            if (ComponentsRegistry* componentsRegistry = GetComponentsRegistry())
            {
                const component_id componentID = componentsRegistry->GetComponentID<ComponentType>();
                return static_cast<ComponentType*>(FindComponent(componentID));
            }
            
            return nullptr;
        }

        template<typename ComponentType>
        void RemoveComponent()
        {
            if (ComponentsRegistry* componentsRegistry = GetComponentsRegistry())
            {
                RemoveComponent(componentsRegistry->GetComponentID<ComponentType>());
            }
        }

        template<typename ComponentType>
        void DeferredRemoveComponent()
        {
            if (ComponentsRegistry* componentsRegistry = GetComponentsRegistry())
            {
                const component_id componentID = componentsRegistry->GetComponentID<ComponentType>();
                DeferredRemoveComponent(componentID);
            }
        }

        inline entity_id id() const { return m_id; }
        inline archetype_id archetypeID() const { return m_archetypeID; }
        inline std::weak_ptr<World> world() const { return m_world;}

        ArchetypesRegistry* GetArchetypesRegistry() const;
        ComponentsRegistry* GetComponentsRegistry() const;
        World* GetWorld() const { return m_world.lock().get();}

    private:
        void AddComponent(component_id componentID);
        void DeferredAddComponent(component_id componentID);
        void* GetComponent(component_id componentID) const;
        void* FindComponent(component_id componentID) const noexcept;
        void RemoveComponent(component_id componentID);
        void DeferredRemoveComponent(component_id componentID);

        entity_id m_id;
        archetype_id m_archetypeID;
        std::weak_ptr<World> m_world;
        std::shared_ptr<BatchComponentActionProcessor> m_batchComponentActionProcessor;
    };
}