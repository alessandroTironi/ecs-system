#pragma once

#include <unordered_map>
#include "Archetypes.h"
#include "PackedComponentArray.h"

namespace ecs
{
    class ArchetypesDatabase
    {
    public:
        ArchetypesDatabase() = default;
        ~ArchetypesDatabase() = default;

        template<typename... Components>
        void AddEntity(entity_id entity)
        {
            AddEntity(entity, { component_data(GetTypeHash(Components), sizeof(Components), 
                ComponentsDatabase::GetComponentID<Components>(), 8)...});
        }

        template<typename ComponentType>
        ComponentType& GetComponent(entity_id entity)
        {
            return *static_cast<ComponentType*>(GetComponent(entity, GetTypeHash(ComponentType)));
        }

        void RemoveEntity(entity_id entity);

        template<typename ComponentType>
        void AddComponent(entity_id entity)
        {
            AddComponent(entity, GetTypeHash(ComponentType));
        }

        template<typename ComponentType>
        void RemoveComponent(entity_id entity)
        {
            RemoveComponent(entity, GetTypeHash(ComponentType));
        }

        const archetype& GetArchetype(entity_id entity);
        size_t GetNumArchetypes() const { return m_archetypesMap.size(); }
        void Reset();

    private:
        struct archetype_set
        {
        public:
            archetype_set() = default;
            archetype_set(const archetype& archetype);

            /* Adds one element to each packed_component_array struct, returning the common index. */
            size_t add_entity(entity_id entity);
            size_t get_entity_index(entity_id entity) const;
            size_t get_num_entities() const { return m_entityToIndexMap.size(); }
            bool try_get_entity_index(entity_id entity, size_t& index) const;
            void* get_component_at_index(const type_hash_t componentHash, const size_t index) const;
            void remove_entity(entity_id entity);
            inline const archetype& get_archetype() const { return m_archetype; }
        private:
            archetype m_archetype;
            std::unordered_map<component_id, std::shared_ptr<packed_component_array_t>> m_componentArraysMap;
            std::unordered_map<entity_id, size_t> m_entityToIndexMap;
            std::unordered_map<size_t, entity_id> m_indexToEntityMap; //@todo replace this with a plain array for cache locality
        };

        std::unordered_map<size_t, archetype_set> m_archetypesMap;
        std::unordered_map<entity_id, size_t> m_entitiesArchetypeHashesMap;

        void AddEntity(entity_id entity, std::initializer_list<component_data> componentTypes);
        void AddEntity(entity_id entity, const archetype& archetype);

        void* GetComponent(entity_id entity, const type_hash_t componentHash);

        void AddComponent(entity_id entity, const type_hash_t componentHash);
        void RemoveComponent(entity_id entity, const type_hash_t componentHash);

        void MoveEntity(entity_id entity, const archetype& targetArchetype);

        void RemoveArchetypeSet(const archetype& archetype);
    };
}