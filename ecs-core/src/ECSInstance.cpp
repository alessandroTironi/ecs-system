#include "ECSInstance.h"
#include "ISystem.h"

#include <assert.h>
#include <iostream>

using namespace ecs;

Instance::Instance(size_t maxEntities)
{
    m_maxEntities = maxEntities;

    m_activeEntities.reserve(m_maxEntities);
    m_availableEntities.reserve(m_maxEntities);

    for (entity_id idx = 0; idx < m_maxEntities; ++idx)
    {
        m_availableEntities.push_back(m_maxEntities - idx);
    }
}

void Instance::Initialize()
{
    // reserving an initial memory block for 30 systems.
    m_registeredSystems.reserve(30);
}

void Instance::Update(real_t deltaTime)
{
    for (auto it = m_registeredSystems.begin(); it != m_registeredSystems.end(); ++it)
    {
        it->second->Update(deltaTime);
    }
}

void Instance::AddSystem(std::shared_ptr<ISystem> system)
{
    const type_hash_t systemHash = GetTypeHash(system.get());
    m_registeredSystems.insert({systemHash, system});
}

void Instance::RemoveSystem(const std::shared_ptr<ISystem>& system)
{
    const type_hash_t systemHash = GetTypeHash(system.get());
    m_registeredSystems.erase(systemHash);
}

void Instance::AddEntity(entity_id& addedEntity)
{
    if (m_availableEntities.size() == 0)
    {
        throw std::exception("Reached maximum allowed amount of entities.");
    }

    addedEntity = m_availableEntities.back();
    m_availableEntities.pop_back();
    m_activeEntities.emplace_back(addedEntity);
}

void Instance::RemoveEntity(entity_id entityToRemove)
{
    for (auto entityIt = m_activeEntities.begin(); entityIt != m_activeEntities.end(); ++entityIt)
    {
        const entity_t& entity = *entityIt;
        if (entity.getUniqueID() == entityToRemove)
        {
            m_activeEntities.erase(entityIt);

            // TODO remove all components

            break;
        }
    }
}