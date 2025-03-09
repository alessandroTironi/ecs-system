#pragma once 

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <typeinfo>
#include <stdexcept>
#include "Types.h"
#include "ComponentArray.h"
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
        const entity_t& GetEntity(entity_id id) const { return m_activeEntities.at(id); }
        void RemoveEntity(entity_id entityToRemove);
        inline size_t GetNumActiveEntities() const { return m_activeEntities.size(); }

        template<typename ComponentType>
        ComponentType& AddComponent(const entity_id entityID)
        {
            const type_hash_t cType = GetTypeHash(ComponentType);
            auto optionalComponentArray = m_componentArraysMap.find(cType);
            std::shared_ptr<component_array<ComponentType>> array = nullptr;
            if (optionalComponentArray == m_componentArraysMap.end())
            {
                array = std::make_shared<component_array<ComponentType>>();
                m_componentArraysMap[cType] = array;
            }
            else
            {
                array = std::static_pointer_cast<component_array<ComponentType>>(
                    optionalComponentArray->second
                );
            }

            return array->add_component(entityID);
        }

        template<typename ComponentType>
        ComponentType& GetComponent(const entity_id entity) const
        {
            const type_hash_t cType = GetTypeHash(ComponentType);
            auto optionalComponentArray = m_componentArraysMap.find(cType);
            std::shared_ptr<component_array<ComponentType>> array = nullptr;
            if (optionalComponentArray == m_componentArraysMap.end())
            {
                throw std::out_of_range("No component of given type found for this entity.");
            }
            else
            {
                array = std::static_pointer_cast<component_array<ComponentType>>(
                    optionalComponentArray->second
                );
            }

            return array->get(entity);
        }

        bool DoesComponentExist(entity_id entity, const type_hash_t componentType) const;

        template<typename ComponentType>
        bool DoesComponentExist(entity_id entity) const 
        { 
            const type_hash_t cType = GetTypeHash(ComponentType);
            return DoesComponentExist(entity, cType);
        }

        size_t GetNumComponents(const type_hash_t componentType) const;

        template<typename ComponentType>
        size_t GetNumComponents() const 
        {
            const type_hash_t cType = GetTypeHash(ComponentType);
            return GetNumComponents(cType);
        }

        void RemoveComponent(const entity_id entity, const type_hash_t componentType);

        template<typename ComponentType> 
        void RemoveComponent(const entity_id entity) 
        {
            const type_hash_t cType = GetTypeHash(ComponentType);
            RemoveComponent(entity, cType);
        }

        void RemoveAllComponents(const entity_id entity);

    protected:
        /* Stores all the Systems registered to this ECS instance. */
        std::unordered_map<type_hash_t, std::shared_ptr<ISystem>> m_registeredSystems;

        std::unordered_map<entity_id, entity_t> m_activeEntities;

        std::vector<entity_id> m_availableEntities;

        std::unordered_map<type_hash_t, std::shared_ptr<component_array_base>> m_componentArraysMap;

    private:
        size_t m_maxEntities;
    };
}