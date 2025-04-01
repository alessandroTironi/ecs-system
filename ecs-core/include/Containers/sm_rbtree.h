#pragma once 

#include <cstdint>
#include <memory>
#include <stdexcept>

namespace ecs
{
    template<typename T>
    struct sm_rbtree
    {
    public:
        sm_rbtree()
        {

        }

        ~sm_rbtree()
        {

        }

        void insert(T value)
        {
            throw std::runtime_error("Not implemented");
        }

        void erase(T value)
        {
            throw std::runtime_error("Not implemented");
        }

        void clear()
        {
            throw std::runtime_error("Not implemented");
        }

        struct iterator
        {
        public:
            iterator(sm_rbtree<T>* inTree, size_t inIndex)
                : m_ownerTree{inTree}, m_index{inIndex}
            {}

            iterator& operator++()
            {
                throw std::runtime_error("Not implemented");
            }

            bool operator==(const iterator& other) const
            {
                throw std::runtime_error("Not implemented");
            }

            bool operator!=(const iterator& other) const
            {
                throw std::runtime_error("Not implemented");
            }

            inline T& value() const
            {
                throw std::runtime_error("Not implemented");
            }

        private:
            sm_rbtree<T>* m_ownerTree;
            size_t m_index;
            size_t m_lastVisited = 0;
        };

        iterator find(T value) const 
        {
            throw std::runtime_error("Not implemented");
        }

        iterator begin() { return iterator(this, 1); }
        iterator end() { return iterator(this, 0); }

        inline size_t size() const noexcept { return m_size; }

    private:
        struct node
        {
            T value;
            size_t parent;
            size_t left;
            size_t right;
            bool red : 1;

            node() = default;
            node(T inValue, size_t inParent, bool inRed = true)
                : value{inValue}, parent{inParent}, red{inRed}
            {}
            node(T inValue, size_t inParent, size_t inLeft, size_t inRight, bool inRed)
                : value{inValue}, parent{inParent}, left{inLeft}, right{inRight}, red{inRed}
            {}
        };

        size_t m_size = 0;

        static thread_local void* s_nodes;
        static thread_local size_t s_usedNodesCount;
        static thread_local size_t* s_freeIndices;
        static thread_local size_t s_freeIndicesCount;
        static thread_local size_t s_capacity;
        static thread_local size_t s_referenceCount;
    };
}

template<typename T>
thread_local void* ecs::sm_rbtree<T>::s_nodes = nullptr;

template<typename T>
thread_local size_t ecs::sm_rbtree<T>::s_usedNodesCount;

template<typename T>
thread_local size_t* ecs::sm_rbtree<T>::s_freeIndices;

template<typename T>
thread_local size_t ecs::sm_rbtree<T>::s_freeIndicesCount;

template<typename T>
thread_local size_t ecs::sm_rbtree<T>::s_capacity;

template<typename T>
thread_local size_t ecs::sm_rbtree<T>::s_referenceCount;
