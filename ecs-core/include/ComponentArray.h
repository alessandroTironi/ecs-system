#pragma once 

#include "Types.h"
#include "Entity.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <bit>
#include "ComponentData.h"

namespace ecs
{
    struct component_array_base
    {
    public:
        using byte = unsigned char;

        component_array_base();
        component_array_base(type_hash_t componentType, size_t sizeOfComponent, size_t maxNumComponents);

        template<typename ComponentType>
        static component_array_base make(size_t maxNumComponents)
        {
            const type_hash_t cType = GetTypeHash(ComponentType);
            return component_array_base(cType, sizeof(ComponentType), maxNumComponents);
        }

        /* Current amount of stored components. */
        inline size_t size() const { return m_indicesMap.size(); }

        byte* allocate_component(const entity_id entityID);

        byte* get_byte_ptr(const entity_id entityID) const;
        byte* find_byte_ptr(const entity_id entityID) const;

        bool free_component(const entity_id entityID);

    protected:
        /* Size (in bytes) of a single instance of the contained component class. */
        size_t m_componentSize = 1;

        /* Maximum allowed amount of components contained in this array. */
        size_t m_maxNumComponents = 1;

        /* Hash of the contained component type. */
        type_hash_t m_componentType = 0;

        /* Pointer to an array containing the needed data. */
        std::unique_ptr<byte[]> m_data;

        /* Maps each entity to the index its component is stored in the byte array. */
        std::unordered_map<entity_id, size_t> m_indicesMap;

        /*  Holds a list of free indices, corresponding to previously occupied array slots. 
            When adding a new component, priority will be given to indices in this list,
            to ensure array packing. */
        std::vector<size_t> m_freeIndices;

        void initialize_containers();
    };

    template<typename ComponentType, size_t NumComponents = 1>
    struct component_array : public component_array_base 
    {
    public:
        component_array() : component_array_base(GetTypeHash(ComponentType),
            sizeof(ComponentType), NumComponents)
        {
            initialize_containers();
        }

        ComponentType& add_component(const entity_id entityID) 
        {
            byte* componentPtr = allocate_component(entityID);
            return *std::bit_cast<ComponentType*>(componentPtr);
        }

        bool find(const entity_id entityID, ComponentType& component) const
        {
            byte* componentPtr = find_byte_ptr(entityID);
            if (componentPtr != nullptr)
            {
                component = *std::bit_cast<ComponentType*>(componentPtr);
                return true;
            }

            return false;
        }

        ComponentType& get(const entity_id entityID) const
        {
            return *std::bit_cast<ComponentType*>(get_byte_ptr(entityID));
        }

        bool remove_component(const entity_id entityID) 
        {
            return free_component(entityID);
        }
    };
}