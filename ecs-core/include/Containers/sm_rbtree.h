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
    private:
        struct node_t;
        struct node_handle_t;

    public:
        using Allocator = SingleBlockFreeListAllocator<node_t, InitialCapacity>;

        sm_rbtree()
        {

        }

        ~sm_rbtree()
        {

        }

        void insert(T value)
        {
            node_t newNode(value, NIL, NIL, NIL, false);
            if (m_rootIndex == NIL)
            {
                // First insertion of in the tree
                const size_t nodeIndex = m_allocator.AllocateBlock();
                node_t& rootNode = m_allocator[nodeIndex];
                rootNode = newNode;

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
                    node_t& currentNode = m_allocator[currentIndex];
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
                newNode.parent = parentIndex;
                newNode.red = true;
                m_allocator[newNodeIndex] = newNode;

                // 2. Validation
                node_t& parentNode = m_allocator[parentIndex];
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

                balance_tree(newNodeIndex);
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
        node_handle get_node(size_t index) const
        {
            return node_handle(index, &m_allocator);
        }

        void left_rotate(node_handle_t node)
        {
            
        }

        void right_rotate(node_handle_t thisNode)
        {
            // get required handles
            node_t& node = thisNode.node();
            node_handle_t leftChild = thisNode.left();
            node_handle_t rightChild = thisNode.right();

            // perform rotation
            leftChild.node().right = thisNode.index;
            leftChild.node().parent = thisNode().parent().index;
            thisNode.node().parent = leftChild.index;
            thisNode.node().left = rightChild.index;
        }

        void balance_tree(const node_handle_t current)
        {
            node_t& currentNode = current.node();
            if (currentNode.parent == NIL)
            {
                // It's root node, no need to rebalance. Just ensure the color is black
                currentNode.red = false;
                return;
            }

            node_handle_t uncle = current.uncle();
            if (!uncle.is_valid())
            {
                // @todo are we sure?
                return;
            }

            node_t& uncleNode = uncle.node();
            node_t& parentNode = current.parent().node();
            if (uncleNode.red)
            {
                // change color of parent and uncle to red
                uncleNode.red = true;
                parentNode.red = true;

                // set grandparent's color to black
                if (parentNode.parent == NIL)
                {
                    return;
                }
                node_t& grandParent = m_allocator[parentNode.parent];
                grandParent.red = true;

                // restart balance from grand parent
                balance_tree(current.grand_parent());

            }
            else
            {

            }
        }
        
        struct node_t
        {
            T value;
            size_t parent;
            size_t left;
            size_t right;
            bool red : 1;

            node_t() = default;
            node_t(T inValue, size_t inParent, bool inRed = true)
                : value{inValue}, parent{inParent}, red{inRed}
            {}
            node_t(T inValue, size_t inParent, size_t inLeft, size_t inRight, bool inRed)
                : value{inValue}, parent{inParent}, left{inLeft}, right{inRight}, red{inRed}
            {}

            inline bool is_right_child(const Allocator& allocator) const 
            {
                if (parent == NIL)
                {
                    return false;
                }

                node_t& parentNode = allocator[parent];
                return parentNode.value <= value;
            }
        };

        struct node_handle_t
        {
            size_t index;
            Allocator* allocator;

            node_handle_t(Allocator* inAllocator) : index{NIL}, allocator{inAllocator} {}
            node_handle_t(size_t inIndex, Allocator* inAllocator)
                : index{inIndex}, allocator{inAllocator}
            {}

            const static node_handle_t NilNode = node_handle_t(NIL, nullptr);
            bool is_valid() const { return index != NIL && allocator != nullptr; }

            inline node_t& node() { return allocator[index]; }
            inline node_handle_t parent() const { return is_valid()? node_handle_t(node().parent, allocator) : NilNode; }
            inline node_handle_t left() const { return node_handle_t(node().left, allocator); }
            inline node_handle_t right() const { return node_handle_t(node().right, allocator); }
            inline node_handle_t grand_parent() const { return parent().parent(); }
            
            node_handle_t sibling() 
            {
                if (parent().is_valid())
                {
                    if (parent().node().value > node().value)
                    {
                        return node_handle_t(parent().node().right, allocator);
                    }
                    else
                    {
                        return node_handle_t(parent().node().left, allocator);
                    }
                } 

                return node_handle_t(allocator);
            }

            inline node_handle_t uncle() const { return parent().sibling(); }
        };

        size_t m_size = 0;
        size_t m_rootIndex = 0;
        
        Allocator m_allocator;
    };
}

#undef NIL