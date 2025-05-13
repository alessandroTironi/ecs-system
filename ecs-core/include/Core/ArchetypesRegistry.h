#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include "Types.h"
#include "Entity.h"
#include "Archetypes.h"
#include "PackedComponentArray.h"
#include "ComponentsRegistry.h"
#include "ComponentData.h"
#include "IDGenerator.h"
#include "BatchComponentActionProcessor.h"
#include "Containers/PoolMemoryAllocator.h"
#include "Containers/SetPoolAllocator.h"
#include "Containers/DynamicBucketAllocators.h"
#include "Containers/memory.h"

namespace ecs
{
    class World;

    class ArchetypesRegistry
    {
        friend class EntityHandle;
        friend class BatchComponentActionProcessor;
    public:
        using ArchetypesSet = pm_set<archetype_id, 128>;

        ArchetypesRegistry() = default;
        ArchetypesRegistry(std::shared_ptr<World> world) : m_world(world) {}

        ~ArchetypesRegistry() = default;

        template<typename... Components>
        void AddEntity(entity_id entity)
        {
            AddEntity(entity, { component_data(sizeof(Components), 
                GetComponentsRegistry()->GetComponentID<Components>(), 8)...});
        }

        template<typename ComponentType>
        ComponentType& GetComponent(entity_id entity)
        {
            return *static_cast<ComponentType*>(GetComponent(entity, GetComponentsRegistry()->GetComponentID<ComponentType>()));
        }

        template<typename ComponentType>
        ComponentType* FindComponent(entity_id entity)
        {
            return static_cast<ComponentType*>(FindComponent(entity, GetComponentsRegistry()->GetComponentID<ComponentType>()));
        }

        void RemoveEntity(entity_id entity);

        template<typename ComponentType>
        void AddComponent(entity_id entity)
        {
            AddComponent(entity, GetComponentsRegistry()->GetComponentID<ComponentType>());
        }

        template<typename ComponentType>
        void RemoveComponent(entity_id entity)
        {
            RemoveComponent(entity, GetComponentsRegistry()->GetComponentID<ComponentType>());
        }

        const archetype& GetArchetype(entity_id entity) const;
        archetype_id GetArchetypeID(entity_id entity) const;
        inline size_t GetNumEntitiesForArchetype(archetype_id archetypeID) const 
        {
            if (archetypeID >= m_archetypeSets.size())
            {
                return 0;
            }

            return m_archetypeSets[archetypeID].get_num_entities();
        }
        
        size_t GetNumArchetypes() const { return m_archetypeSets.size(); }
        void Reset();

        /** 
         * @brief Calls the provided function over all the entities that have the given components. 
         * 
         * @param function The function to call for each entity.
         * @param components The components to query for.
         */
        template<typename... Components>
        void ForEachEntity(std::function<void(EntityHandle, Components&...)> function)
        {
            ArchetypesSet archetypes;
            QueryArchetypes({ GetComponentsRegistry()->GetComponentID<Components>()... }, archetypes);

            std::shared_ptr<BatchComponentActionProcessor> batchComponentActionProcessor =
                std::make_shared<BatchComponentActionProcessor>(m_world);
            for (const archetype_id archetypeID : archetypes) 
            {
                const archetype_set& archetypeSet = m_archetypeSets[archetypeID];
                const auto& indexMap = archetypeSet.index_map();
                
                // Only iterate over valid indices in the index map
                for (const auto& [entityIndex, entityId] : indexMap)
                {
                    EntityHandle handle = EntityHandle(m_world, entityId, archetypeID, batchComponentActionProcessor);
                    function(handle, *static_cast<Components*>(archetypeSet.get_component_at_index(GetComponentsRegistry()->GetComponentID<Components>(), entityIndex))...);
                }
            }

            batchComponentActionProcessor->ProcessActions();
        }

        void QueryEntities(std::initializer_list<component_id> components, std::vector<entity_id>& entities);

    private:
        struct archetype_set
        {
        public:
            archetype_set();
            archetype_set(const archetype& archetype, ComponentsRegistry* componentsRegistry);

            /* Adds one element to each packed_component_array struct, returning the common index. */
            size_t add_entity(entity_id entity);
            size_t get_entity_index(entity_id entity) const;
            size_t get_num_entities() const { return m_entityToIndexMap.size(); }
            bool try_get_entity_index(entity_id entity, size_t& index) const;
            void* get_component_at_index(const component_id componentID, const size_t index) const;
            void* find_component_at_index(const component_id componentID, const size_t index) const;
            void remove_entity(entity_id entity);
            void copy_entity_to(const entity_id entity, archetype_set& destination);
            inline const archetype& get_archetype() const { return m_archetype; }
            inline const pm_unordered_map<entity_id, size_t, MAX_ENTITIES, MAX_ENTITIES>& entity_map() const 
            { 
                return m_entityToIndexMap; 
            }

            inline const entity_id get_entity_at_index(const size_t index) const { return m_indexToEntityMap.at(index);}
            inline const pm_unordered_map<size_t, entity_id, MAX_ENTITIES, MAX_ENTITIES>& index_map() const 
            { 
                return m_indexToEntityMap; 
            }
        
        private:
            archetype m_archetype;
            pm_unordered_map<component_id, std::shared_ptr<packed_component_array_t>, 
                MAX_COMPONENTS, MAX_COMPONENTS> m_componentArraysMap;
            pm_unordered_map<entity_id, size_t, MAX_ENTITIES, MAX_ENTITIES> m_entityToIndexMap;
            pm_unordered_map<size_t, entity_id, MAX_ENTITIES, MAX_ENTITIES> m_indexToEntityMap; //@todo replace this with a plain array for cache locality
        };

        void AddEntity(entity_id entity, std::initializer_list<component_data> componentTypes);
        void AddEntity(entity_id entity, const archetype& archetype);

        void* GetComponent(entity_id entity, const component_id componentID);
        void* FindComponent(entity_id entity, const component_id componentID);

        void AddComponent(entity_id entity, const type_key& componentType);
        void AddComponent(entity_id entity, const component_id componentID);
        void RemoveComponent(entity_id entity, const type_key& componentType);
        void RemoveComponent(entity_id entity, const component_id componentID);

        void MoveEntity(entity_id entity, const archetype& targetArchetype);

        archetype_id GetOrCreateArchetypeID(const archetype& archetype);
        archetype_set& GetOrCreateArchetypeSet(const archetype& archetype);

        void QueryArchetypes(std::initializer_list<component_id> components, ArchetypesSet& foundArchetypes);

        ComponentsRegistry* GetComponentsRegistry() const; 
        World* GetWorld() const;

        /* A map of archetypes to their IDs. */
        pm_unordered_map<archetype, archetype_id, MAX_ENTITIES, MAX_ENTITIES> m_archetypesIDMap;
        pm_unordered_map<entity_id, archetype_id, MAX_ENTITIES, MAX_ENTITIES> m_entitiesArchetypeHashesMap; 

        /* Generator for unique archetype IDs.*/
        IDGenerator<archetype_id> m_archetypeIDGenerator;

        /* A vector of all the registered archetype sets which actually acts as a hash table where the key is the archetype's ID. 
           Since archetype IDs are generated sequentially, this hash table is guaranteed to be collision-free and compact. */
        std::vector<archetype_set> m_archetypeSets;

        /** 
         * Maps each component ID to a vector made of the IDs of any archetype that contains that component.
         * This is used by the query system to find all the archetypes that contain the given component.
         */
        pm_unordered_map<component_id, ArchetypesSet, MAX_COMPONENTS, MAX_COMPONENTS> m_componentToArchetypeSetMap;

        /* A reference to the world. */
        std::shared_ptr<World> m_world;
    };
}