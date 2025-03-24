#pragma once

#include <unordered_map>
#include <memory>
#include "Types.h"
#include "Entity.h"
#include "Archetypes.h"
#include "PackedComponentArray.h"
#include "ComponentsRegistry.h"
#include "IDGenerator.h"

namespace ecs
{
    class World;

    class ArchetypesRegistry
    {
        friend class EntityHandle;
    public:
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
        
        size_t GetNumArchetypes() const { return m_archetypeSets.size(); }
        void Reset();

        template<typename... Components>
        void ForEachEntity(std::function<void(EntityHandle, Components&...)> function)
        {
            std::set<archetype_id> archetypes;
            QueryArchetypes({ GetComponentsRegistry()->GetComponentID<Components>()... }, archetypes);

            for (const archetype_id archetypeID : archetypes) 
            {
                // We iterate over all the entities in each archetype by reverse index, rather than by
                // entity ID. This is not just more cache friendly, but it also allows dynamic 
                // edits on the components of each entity. 
                // This still does not prevent an entity from being evaluated multiple or zero times, so
                // @todo we need a defer function for handling these cases.

                const archetype_set& archetypeSet = m_archetypeSets[archetypeID];
                for (size_t entityIndex = archetypeSet.get_num_entities(); entityIndex >= 0; --entityIndex)
                {
                    const size_t numEntities = archetypeSet.get_num_entities();
                    if (entityIndex >= numEntities)
                    {
                        continue;
                    }

                    const entity_id entity = archetypeSet.get_entity_at_index(entityIndex);
                    EntityHandle handle = EntityHandle(m_world, entity, archetypeID);
                    function(handle, *static_cast<Components*>(archetypeSet.get_component_at_index(GetComponentsRegistry()->GetComponentID<Components>(), entityIndex))...);
                
                }
            }
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
            inline const archetype& get_archetype() const { return m_archetype; }
            inline const std::unordered_map<entity_id, size_t>& entity_map() const { return m_entityToIndexMap; }
            inline const entity_id get_entity_at_index(const size_t index) const { return m_indexToEntityMap.at(index);}
        
        private:
            archetype m_archetype;
            std::unordered_map<component_id, std::shared_ptr<packed_component_array_t>> m_componentArraysMap;
            std::unordered_map<entity_id, size_t> m_entityToIndexMap;
            std::unordered_map<size_t, entity_id> m_indexToEntityMap; //@todo replace this with a plain array for cache locality
        };

        void AddEntity(entity_id entity, std::initializer_list<component_data> componentTypes);
        void AddEntity(entity_id entity, const archetype& archetype);

        void* GetComponent(entity_id entity, const component_id componentID);
        void* FindComponent(entity_id entity, const component_id componentID);

        void AddComponent(entity_id entity, const name& componentName);
        void AddComponent(entity_id entity, const component_id componentID);
        void RemoveComponent(entity_id entity, const name& componentName);
        void RemoveComponent(entity_id entity, const component_id componentID);

        void MoveEntity(entity_id entity, const archetype& targetArchetype);

        archetype_id GetOrCreateArchetypeID(const archetype& archetype);
        archetype_set& GetOrCreateArchetypeSet(const archetype& archetype);

        void QueryArchetypes(std::initializer_list<component_id> components, std::set<archetype_id>& foundArchetypes);

        ComponentsRegistry* GetComponentsRegistry() const; 
        World* GetWorld() const;

        /* A map of archetypes to their IDs. */
        std::unordered_map<archetype, archetype_id> m_archetypesIDMap;
        std::unordered_map<entity_id, archetype_id> m_entitiesArchetypeHashesMap; 

        /* Generator for unique archetype IDs.*/
        IDGenerator<archetype_id> m_archetypeIDGenerator;

        /* A vector of all the registered archetype sets which actually acts as a hash table where the key is the archetype's ID. 
           Since archetype IDs are generated sequentially, this hash table is guaranteed to be collision-free and compact. */
        std::vector<archetype_set> m_archetypeSets;

        /** 
         * Maps each component ID to a vector made of the IDs of any archetype that contains that component.
         * This is used by the query system to find all the archetypes that contain the given component.
         */
        std::unordered_map<component_id, std::set<archetype_id>> m_componentToArchetypeSetMap;

        /* A reference to the world. */
        std::shared_ptr<World> m_world;
    };
}