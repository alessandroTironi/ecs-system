#pragma once 

#include <memory>
#include <limits>
#include "Types.h"
#include "ComponentData.h"
#include "World.h"
#include "ArchetypesRegistry.h"

namespace ecs
{
    class ComponentsRegistry;

    class EntityHandle
    {
    public:
        EntityHandle();
        EntityHandle(std::weak_ptr<World> world, entity_id id, archetype_id archetypeID);

        template<typename ComponentType>
        void AddComponent()
        {
            if (ArchetypesRegistry* archetypesRegistry = GetArchetypesRegistry())
            {
                archetypesRegistry->AddComponent<ComponentType>(m_id);
            }
        }

        template<typename ComponentType>
        ComponentType& GetComponent()
        {
            throw std::runtime_error("Not implemented");
        }

        template<typename ComponentType>
        void RemoveComponent()
        {
            throw std::runtime_error("Not implemented");
        }

        inline entity_id id() const { return m_id; }
        inline archetype_id archetypeID() const { return m_archetypeID; }
        inline std::weak_ptr<World> world() const { return m_world;}

    private:
        void AddComponent(component_id componentID);
        void* GetComponent(component_id componentID);
        void RemoveComponent(component_id componentID);

        ArchetypesRegistry* GetArchetypesRegistry() const 
        {
            if (!m_world.expired())
            {
                return m_world.lock().get()->GetArchetypesRegistry();
            }

            return nullptr;
        }

        entity_id m_id;
        archetype_id m_archetypeID;
        std::weak_ptr<World> m_world;
    };
}