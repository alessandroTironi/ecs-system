#include "ECSInstance.h"
#include "ISystem.h"

using namespace ecs;

Instance::Instance()
{

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