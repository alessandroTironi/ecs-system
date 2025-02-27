#pragma once 

#include "Types.h"

#include <stdexcept>

namespace ecs
{
    struct component_array_base {};

    struct IComponent {};

    template<typename ComponentType, size_t MaxSize = 1000>
    struct component_array : public component_array_base
    {
    public:
        component_array()
        {
            m_indicesMap.reserve(MaxSize);
            m_freeIndicesList.reserve(MaxSize);
        }

        inline size_t capacity() const { return MaxSize; }

        ComponentType& add_component(entity_id entity)
        {
            // @TODO support to multiple components of same type for the same entity

            if (m_indicesMap.find(entity) != m_indicesMap.end())
            {
                throw std::invalid_argument("Multiple components of same type for the same entity are not currently supported.");
            }

            size_t nextIndex = m_indicesMap.size();
            if (nextIndex >= MaxSize)
            {
                throw std::exception("Reached maximum allowed amount of components of this type.");
            }

            if (m_freeIndicesList.size() > 0)
            {
                // give priority to recently freed indices
                nextIndex = m_freeIndicesList.back();
                m_freeIndicesList.pop_back();
            }

            m_components[nextIndex] = ComponentType();
            m_indicesMap[entity] = nextIndex;

            return m_components[nextIndex];
        }

        bool remove_component(entity_id entity)
        {
            if (m_indicesMap.find(entity) == m_indicesMap.end())
            {
                return false;
            }

            const size_t index = m_indicesMap[entity]; // @TODO this is a double lookup, find a way to avoid it and reuse result from find()
            m_freeIndicesList.push_back(index);

            m_indicesMap.erase(entity);
            return true;
        }

        ComponentType& get_component(entity_id entity)
        {
            return m_components[m_indicesMap.at(entity)];
        }

        inline size_t GetNumComponents() const { return m_indicesMap.size(); }

    private:
        std::array<ComponentType, MaxSize> m_components;
        std::unordered_map<entity_id, size_t> m_indicesMap;
        std::vector<size_t> m_freeIndicesList;
    };
}