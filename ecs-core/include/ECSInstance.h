#pragma once 

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <typeinfo>
#include <stdexcept>
#include "Types.h"
#include "Entity.h"

namespace ecs 
{
    class ISystem;

    class Instance 
    {
    public:
        Instance(size_t maxEntities = 1000);
        ~Instance() = default;

        void Initialize();
        void Update(real_t deltaTime);

        inline size_t GetMaxNumEntities() const { return m_maxEntities; }

        /* Registers the provided system to this ECS instance. */
        void AddSystem(std::shared_ptr<ISystem> system);

        /* Creates an instance of the provided system class and registers it. */
        template<typename SystemType>
        std::shared_ptr<SystemType> AddSystem()
        {
            std::shared_ptr<SystemType> system = std::make_shared<SystemType>() ;
            AddSystem(system);
            return system;
        }

        /* Removes the system provided as argument. */
        void RemoveSystem(const std::shared_ptr<ISystem>& system);

        inline size_t GetSystemsCount() const { return m_registeredSystems.size(); }

        void AddEntity(entity_id& addedEntity);
        const EntityHandle& GetEntity(entity_id id) const { return m_activeEntities.at(id); }
        void RemoveEntity(entity_id entityToRemove);
        inline size_t GetNumActiveEntities() const { return m_activeEntities.size(); }

    protected:
        /* Stores all the Systems registered to this ECS instance. */
        std::unordered_map<size_t, std::shared_ptr<ISystem>> m_registeredSystems;

        std::unordered_map<entity_id, EntityHandle> m_activeEntities;

        std::vector<entity_id> m_availableEntities;

    private:
        size_t m_maxEntities;
    };
}