#pragma once 

#include <memory>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include "Types.h"

namespace ecs 
{
    class ISystem;

    class Instance 
    {
    public:
        Instance();
        ~Instance() = default;

        void Initialize();
        void Update(real_t deltaTime);

        /* Registers the provided system to this ECS instance. */
        void AddSystem(std::shared_ptr<ISystem> system);

        /* Creates an instance of the provided system class and registers it. */
        template<typename SystemType>
        std::shared_ptr<SystemType> AddSystem()
        {
            std::shared_ptr<SystemType> system = std::shared_ptr<SystemType>(new SystemType());
            AddSystem(system);
            return system;
        }

        /* Removes the system provided as argument. */
        void RemoveSystem(const std::shared_ptr<ISystem>& system);

        inline size_t GetSystemsCount() const { return m_registeredSystems.size(); }

    protected:
        /* Stores all the Systems registered to this ECS instance. */
        std::unordered_map<type_hash_t, std::shared_ptr<ISystem>> m_registeredSystems;
    };
}