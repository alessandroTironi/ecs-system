#include "Core/BatchComponentActionProcessor.h"
#include "Core/World.h"
#include "Core/ArchetypesRegistry.h"

using namespace ecs;

void BatchComponentActionProcessor::AddAction(EBatchComponentActionType type, entity_id entity, component_id component)
{
    m_actions.emplace_back(type, entity, component);
}

void BatchComponentActionProcessor::ProcessActions()
{
    if (m_world.expired())
    {
        return;
    }

    std::shared_ptr<ArchetypesRegistry> registry = m_world.lock()->GetArchetypesRegistry();
    if (registry.get() == nullptr)
    {
        return;
    }

    for (const action& action : m_actions)
    {
        switch(action.actionType)
        {
            case EBatchComponentActionType::Add: 
                registry->AddComponent(action.entity, action.component);
                break;
            case EBatchComponentActionType::Remove:
                registry->RemoveComponent(action.entity, action.component);
                break;
            case EBatchComponentActionType::None:
            default:
                break;
        }
    }

    m_actions.clear();
}
