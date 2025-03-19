#pragma once

#include <unordered_map>
#include <memory>
#include "Archetypes.h"
#include "PackedComponentArray.h"
#include "ComponentsDatabase.h"

namespace ecs
{
    class ArchetypesDatabase
    {
    public:
        ArchetypesDatabase() = default;
        ArchetypesDatabase(std::shared_ptr<ComponentsDatabase> componentsRegistry)
            : m_componentsRegistry(componentsRegistry)
        {}

        ~ArchetypesDatabase() = default;

        template<typename... Components>
        void AddEntity(entity_id entity)
        {
            AddEntity(entity, { component_data(sizeof(Components), 
                m_componentsRegistry->GetComponentID<Components>(), 8)...});
        }

        template<typename ComponentType>
        ComponentType& GetComponent(entity_id entity)
        {
            return *static_cast<ComponentType*>(GetComponent(entity, m_componentsRegistry->GetComponentID<ComponentType>()));
        }

        void RemoveEntity(entity_id entity);

        template<typename ComponentType>
        void AddComponent(entity_id entity)
        {
            AddComponent(entity, m_componentsRegistry->GetComponentID<ComponentType>());
        }

        template<typename ComponentType>
        void RemoveComponent(entity_id entity)
        {
            RemoveComponent(entity, m_componentsRegistry->GetComponentID<ComponentType>());
        }

        const archetype& GetArchetype(entity_id entity);
        size_t GetNumArchetypes() const { return m_archetypesMap.size(); }
        void Reset();

    private:
        struct archetype_set
        {
        public:
            archetype_set() = default;
            archetype_set(const archetype& archetype, ComponentsDatabase* componentsRegistry);

            /* Adds one element to each packed_component_array struct, returning the common index. */
            size_t add_entity(entity_id entity);
            size_t get_entity_index(entity_id entity) const;
            size_t get_num_entities() const { return m_entityToIndexMap.size(); }
            bool try_get_entity_index(entity_id entity, size_t& index) const;
            void* get_component_at_index(const component_id componentID, const size_t index) const;
            void remove_entity(entity_id entity);
            inline const archetype& get_archetype() const { return m_archetype; }
        private:
            archetype m_archetype;
            std::unordered_map<component_id, std::shared_ptr<packed_component_array_t>> m_componentArraysMap;
            std::unordered_map<entity_id, size_t> m_entityToIndexMap;
            std::unordered_map<size_t, entity_id> m_indexToEntityMap; //@todo replace this with a plain array for cache locality
        };

        void AddEntity(entity_id entity, std::initializer_list<component_data> componentTypes);
        void AddEntity(entity_id entity, const archetype& archetype);

        void* GetComponent(entity_id entity, const component_id componentID);

        void AddComponent(entity_id entity, const name& componentName);
        void AddComponent(entity_id entity, const component_id componentID);
        void RemoveComponent(entity_id entity, const name& componentName);
        void RemoveComponent(entity_id entity, const component_id componentID);

        void MoveEntity(entity_id entity, const archetype& targetArchetype);

        void RemoveArchetypeSet(const archetype& archetype);

        std::unordered_map<size_t, archetype_set> m_archetypesMap;
        std::unordered_map<entity_id, size_t> m_entitiesArchetypeHashesMap;

        /* A reference to the world's components registry. */
        std::shared_ptr<ComponentsDatabase> m_componentsRegistry;
    };
}