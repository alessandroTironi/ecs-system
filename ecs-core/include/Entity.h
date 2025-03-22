#pragma once 

#include <memory>
#include <limits>
#include "Types.h"
#include "ComponentData.h"

namespace ecs
{
    class World;

    typedef size_t entity_id;
    const static entity_id INVALID_ENTITY_ID = std::numeric_limits<entity_id>::max();

    class EntityHandle
    {
    public:
        EntityHandle();
        EntityHandle(std::weak_ptr<World> world, entity_id id, archetype_id archetypeID);

        template<typename ComponentType>
        void AddComponent()
        {
            throw std::runtime_error("Not implemented");
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

        entity_id m_id;
        archetype_id m_archetypeID;
        std::weak_ptr<World> m_world;
    };
}