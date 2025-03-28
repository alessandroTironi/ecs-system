#pragma once 

#include <memory>
#include "Types.h"
#include "ComponentData.h"
#include "ComponentsRegistry.h"

namespace ecs
{
    class World;

    /**
     * @brief The types of actions that can be performed by a BatchComponentActionProcessor.
     */
    enum class EBatchComponentActionType : unsigned char 
    {
        None,

        /**
         * @brief Add a component to an entity.
         */
        Add,

        /**
         * @brief Remove a component from an entity.
         */
        Remove
    };

    /**
     * @brief This class stores a list of actions involving components addition and removal
     * that can be batched and processed in one single moment. 
     */
    class BatchComponentActionProcessor
    {
    public:
        BatchComponentActionProcessor() = delete;
        BatchComponentActionProcessor(std::weak_ptr<World> world) : m_world(world) 
        {
            m_actions.reserve(64);
        }

        void AddAction(EBatchComponentActionType type, entity_id entity, component_id component);
        void ProcessActions();

    private:
        struct action 
        {
            action() = default;
            action(EBatchComponentActionType type, entity_id entityID, component_id componentID)
                : actionType(type), entity(entityID), component(componentID) {}

            EBatchComponentActionType actionType;
            entity_id entity; 
            component_id component;
        };

        std::weak_ptr<World> m_world;
        std::vector<action> m_actions;
    };
}