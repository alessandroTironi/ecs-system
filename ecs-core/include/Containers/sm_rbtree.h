#pragma once 

#include <cstdint>
#include <memory>
#include <stdexcept>
#include "SingleBlockFreeListAllocator.h"

#define NIL 0 

namespace ecs
{
    template<typename T, size_t InitialCapacity = 8>
    struct sm_rbtree
    {
    public:
        struct node;

        using Allocator = SingleBlockFreeListAllocator<node, InitialCapacity>;

        sm_rbtree()
        {

        }

        ~sm_rbtree()
        {

        }

        void insert(T value)
        {
            if (m_rootIndex == NIL)
            {
                // First insertion of in the tree
                const size_t nodeIndex = m_allocator.AllocateBlock();
                node& rootNode = m_allocator[nodeIndex];
                rootNode.parent = NIL;
                rootNode.left = NIL;
                rootNode.right = NIL;
                rootNode.red = false;

                m_size += 1;
            }
            else
            {
                // 1. Insert the node as in a normal BST, coloring it red.
                size_t parentIndex = NIL;
                size_t currentIndex = m_rootIndex;
                while (currentIndex != NIL)
                {
                    parentIndex = currentIndex;
                    node& currentNode = m_allocator[currentIndex];
                    if (value > currentNode.value)
                    {
                        currentIndex = currentNode.right;
                    }
                    else if (value == currentNode.value)
                    {
                        // already in tree, return
                        return;
                    }
                    else
                    {
                        currentIndex = currentNode.left;
                    }
                }

                const size_t newNodeIndex = m_allocator.AllocateBlock();
                node& newNode = m_allocator[newNodeIndex];
                newNode.value = value;
                newNode.parent = parentIndex;
                newNode.left = NIL;
                newNode.right = NIL;
                newNode.red = true;

                // 2. Validation
                node& parentNode = m_allocator[parentIndex];
                if (newNode.value > parentNode.value)
                {
                    parentNode.right = newNodeIndex;
                }
                else
                {
                    parentNode.left = newNodeIndex;
                }

                m_size += 1;

                if (!parentNode.red)
                {
                    // no need to rebalance
                    return;
                }

                // 3. Rebalance
                size_t uncleNodeIndex;
                if (!try_get_uncle_node(newNodeIndex, uncleNodeIndex))
                {
                    return;
                }

                node& uncleNode = m_allocator[uncleNodeIndex];
                if (uncleNode.red)
                {
                    uncleNode.red = false;
                }
                else if (uncleNode.is_right_child(m_allocator))
                {
                    left_rotate(uncleNodeIndex);
                }
                else
                {
                    right_rotate(uncleNodeIndex);
                }
            }
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

        bool is_valid() const noexcept 
        {
            throw std::runtime_error("Not implemented");
        }

    private:
        void left_rotate(size_t index)
        {
            throw std::runtime_error("Not implemented");
        }

        void right_rotate(size_t index)
        {
            throw std::runtime_error("Not implemented");
        }

        bool try_get_uncle_node(const size_t childIndex, size_t& outUncleIndex) const noexcept 
        {
            node& childNode = m_allocator[childIndex];
            if (childNode.parent == NIL)
            {
                return false;
            }

            node& parentNode = m_allocator[childNode.parent];
            if (parentNode.parent == NIL)
            {
                return false;
            }

            node& grandParentNode = m_allocator[parentNode.parent];
            if (grandParentNode.left == childNode.parent)
            {
                outUncleIndex = grandParentNode.right;
            }
            else
            {
                outUncleIndex = grandParentNode.left;
            }

            return true;
        }
        
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

            inline bool is_right_child(const Allocator& allocator) const 
            {
                if (parent == NIL)
                {
                    return false;
                }

                node& parentNode = allocator[parent];
                return parentNode.value <= value;
            }
        };

        size_t m_size = 0;
        size_t m_rootIndex = 0;
        
        Allocator m_allocator;
    };
}

#undef NIL