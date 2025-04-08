#pragma once 

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <limits>
#include "SingleBlockFreeListAllocator.h"

namespace ecs
{
    enum class rbtreecolors : uint8_t
    {
        BLACK,
        RED
    };

    template<typename T, size_t InitialCapacity = 8>
    struct rbtree
    {
    private:
        struct node_t;
        struct node_handle_t;

    public:
        using Allocator = SingleBlockFreeListAllocator<node_t, InitialCapacity>;

        static constexpr size_t NIL = std::numeric_limits<size_t>::max();

        rbtree()
        {
            m_rootIndex = NIL;
        }

        ~rbtree()
        {

        }

        void insert(T value)
        {
            node_handle_t y = node_handle_t::NilNode;
            node_handle_t x = get_node(m_rootIndex);

            while (x.is_valid())
            {
                y = x;
                const T xValue = x.node().value;
                if (value < xValue)
                {
                    x = x.left();
                }
                else if (value > xValue)
                {
                    x = x.right();
                }
                else
                {
                    return;
                }
            }

            node_handle_t newNode = allocate_node(value);
            newNode.set_parent(y);
            if (!y.is_valid())
            {
                m_rootIndex = newNode.index;
            }
            else if (newNode.node().value < y.node().value)
            {
                y.set_left(newNode);
            }
            else 
            {
                y.set_right(newNode);
            }

            fixup_insert(newNode);
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
            iterator(rbtree<T>* inTree, size_t inIndex)
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

            node_handle_t get_node_handle() const 
            {
                return m_ownerTree->get_node(m_index);
            }

        private:
            rbtree<T>* m_ownerTree;
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
        node_handle_t binary_search(T value) const
        {
            if (m_size == 0)
            {
                return node_handle_t::NilNode;
            }

            node_handle_t current = get_node(m_rootIndex);
            while (current.is_valid())
            {
                const T currentValue = current.node().value;
                if (currentValue == value)
                {
                    break;
                }
                else if (value > currentValue)
                {
                    current = current.right();
                }
                else
                {
                    current = current.left();
                }
            }

            return current;
        }

        node_handle_t find_minimum(node_handle_t root) const
        {
            node_handle_t current = root;
            while (current.is_valid())
            {
                if (current.left().is_valid())
                {
                    current = current.left();
                }
                else
                {
                    break;
                }
            }

            return current;
        }

        node_handle_t get_node(size_t index) const
        {
            if (index == NIL)
            {
                return node_handle_t::NilNode;
            }

            return node_handle_t(index, &m_allocator);
        }

        void left_rotate(node_handle_t x)
        {
            if (!x.is_valid() || !x.right().is_valid())
            {
                return;
            }

            node_handle_t y = x.right();
            x.set_right(y.left());
            if (y.left().is_valid())
            {
                y.left().set_parent(x);
            }

            y.set_parent(x.parent());
            if (!x.parent().is_valid())
            {
                m_rootIndex = y.index;
            }
            else if (x.is_left_child())
            {
                x.parent().set_left(y);
            }
            else
            {
                x.parent().set_right(y);
            }

            y.set_left(x);
            x.set_parent(y);
        }

        void right_rotate(node_handle_t y)
        {
            if (!y.is_valid() || !y.left().is_valid())
            {
                return;
            }

            node_handle_t x = y.left();
            y.set_left(x.right());
            if (x.right().is_valid())
            {
                x.right().set_parent(y);
            }

            x.set_parent(y.parent());
            if (!y.parent().is_valid())
            {
                m_rootIndex = y.index;
            }
            else if (y.is_left_child())
            {
                y.parent().set_left(x);
            }
            else
            {
                y.parent().set_right(x);
            }

            x.set_right(y);
            y.set_parent(x);
        }

        /**
         * @brief Fixes any violation occurred in the tree after inserting a new node.
         * 
         * @param z Node from which the fixup is started.
         */
        void fixup_insert(node_handle_t z)
        {
            while (z.index != m_rootIndex && z.parent().is_red())
            {
                if (z.parent().is_left_child())
                {
                    node_handle_t y = z.grand_parent().right();
                    if (y.is_valid() && y.is_red())
                    {
                        z.parent().set_color(rbtreecolors::BLACK);
                        y.set_color(rbtreecolors::BLACK);
                        z.grand_parent().set_color(rbtreecolors::RED);
                        z = z.grand_parent();
                    }
                    else
                    {
                        if (z.is_right_child())
                        {
                            z = z.parent();
                            left_rotate(z);
                        }

                        z.parent().set_color(rbtreecolors::BLACK);
                        z.grand_parent().set_color(rbtreecolors::RED);
                        right_rotate(z.grand_parent());
                    }
                }
                else
                {
                    node_handle_t y = z.grand_parent().left();
                    if (y.is_valid() && y.is_red())
                    {
                        z.parent().set_color(rbtreecolors::BLACK);
                        y.set_color(rbtreecolors::BLACK);
                        z.grand_parent().set_color(rbtreecolors::RED);
                        z = z.grand_parent();
                    }
                    else
                    {
                        if (z.is_left_child())
                        {
                            z = z.parent();
                            right_rotate(z);
                        }

                        z.parent().set_color(rbtreecolors::BLACK);
                        z.grand_parent().set_color(rbtreecolors::RED);
                        left_rotate(z.grand_parent());
                    }
                }
            }       
            
            // finally ensure the root is always black
            node_handle_t root = get_node(m_rootIndex);
            if (root.is_valid())
            {
                root.set_color(rbtreecolors::BLACK);
            }
        }

        void transplant(node_handle_t u, node_handle_t v)
        {
            if (!u.parent().is_valid())
            {
                m_rootIndex = v.index;
            }
            else if (u.is_left_child())
            {
                u.parent().set_left(v);
            }
            else
            {
                u.parent().set_right(v);
            }

            if (v.is_valid())
            {
                v.set_parent(u.parent());
            }
        }

        void fixup_erase(node_handle_t x)
        {
            while (x.index != m_rootIndex && x.is_valid() && x.is_black())
            {
                if (x.is_left_child())
                {
                    node_handle_t w = x.parent().right();
                    if (w.is_red())
                    {
                        w.set_color(rbtreecolors::BLACK);
                        x.parent().set_color(rbtreecolors::RED);
                        left_rotate(x.parent());
                        w = x.parent().right();
                    }

                    if ((!w.left().is_valid() || w.left().is_black())
                        && (!w.right().is_valid() || w.right().is_black()))
                    {
                        w.set_color(rbtreecolors::RED);
                        x = x.parent();
                    }
                    else
                    {
                        if (!w.right().is_valid() || w.right().is_black())
                        {
                            if (w.left().is_valid())
                            {
                                w.left().set_color(rbtreecolors::BLACK);
                            }

                            w.set_color(rbtreecolors::RED);
                            right_rotate(w);
                            w = x.parent().right();
                        }

                        w.set_color(x.parent().node().color);
                        x.parent().set_color(rbtreecolors::BLACK);
                        if (w.right().is_valid())
                        {
                            w.right().set_color(rbtreecolors::BLACK);
                        }
                        left_rotate(x.parent());
                        x = get_node(m_rootIndex);
                    }
                }
                else
                {
                    node_handle_t w = x.parent().left();
                    if (w.is_red())
                    {
                        w.set_color(rbtreecolors::BLACK);
                        x.parent().set_color(rbtreecolors::RED);
                        right_rotate(x.parent());
                        w = x.parent().left();
                    }

                    if ((!w.right().is_valid() || w.right().is_black()) &&
                        (!w.left().is_valid() || w.left().is_black()))
                    {
                        w.set_color(rbtreecolors::RED);
                        x = x.parent();
                    }
                    else
                    {
                        if (!w.left().is_valid() || w.left().is_black())
                        {
                            if (w.right().is_valid())
                            {
                                w.right().set_color(rbtreecolors::BLACK);
                            }

                            w.set_color(rbtreecolors::RED);
                            left_rotate(w);
                            w = x.parent().left();
                        }

                        w.set_color(x.parent().node().color);
                        x.parent().set_color(rbtreecolors::BLACK);
                        if (w.left().is_valid())
                        {
                            w.left().set_color(rbtreecolors::BLACK);
                        }

                        right_rotate(x.parent());
                        x = get_node(m_rootIndex);
                    }
                }
            }

            if (x.is_valid())
            {
                x.set_color(rbtreecolors::BLACK);
            }
        }

        /**
         * @brief Utility to find the minimum element smaller to the provided one.
         * 
         * @param node the node whose minimum must be found.
         * @return node_handle_t a handle to the node containing the minimum value.
         */
        node_handle_t find_minimum(node_handle_t node)
        {
            while (node.left().is_valid())
            {
                node = node.left();
            }

            return node;
        }
        
    private:

#pragma region Debug Methods
        void get_leaf_nodes(node_handle_t rootNode, std::vector<node_handle_t>& outLeafNodes) const
        {
            if (rootNode.is_valid() && rootNode.is_leaf())
            {
                outLeafNodes.push_back(rootNode);
            }
            else
            {
                if (rootNode.left().is_valid())
                {
                    get_leaf_nodes(rootNode.left(), outLeafNodes);
                }

                if (rootNode.right().is_valid())
                {
                    get_leaf_nodes(rootNode.right(), outLeafNodes);
                }
            }
        }

        size_t count_black_nodes(node_handle_t from, node_handle_t to) const
        {
            const T fromValue = from.node().value;
            const T toValue = to.node().value;
            const size_t thisNode = from.is_black()? 1 : 0;
            if (fromValue == toValue)
            {
                return thisNode;
            }
            else if (toValue > fromValue)
            {
                return thisNode + count_black_nodes(from.right(), to);
            }   
            else
            {
                return thisNode + count_black_nodes(from.left(), to);
            }
        }

        bool is_valid_subtree(node_handle_t rootNode) const 
        {
            if (rootNode.is_red() && (rootNode.left().is_red() || rootNode.right().is_red()))
            {
                // error: if a node is red, both its children must be black
                return false;
            }

            // get all leaf nodes from here
            std::vector<node_handle_t> leafNodes;
            get_leaf_nodes(rootNode, leafNodes);

            // ensure 
            if (leafNodes.size() == 0)
            {
                return true;
            }

            const size_t requiredNumBlackNodes = count_black_nodes(rootNode, leafNodes[0]);
            for (size_t i = 1; i < leafNodes.size(); ++i)
            {
                const size_t numBlackNodes = count_black_nodes(rootNode, leafNodes[i]);
                if (numBlackNodes != requiredNumBlackNodes)
                {
                    // error: not all the path from this node to any leaf has the same amount of black nodes
                    return false;
                }
            }

            return true;
        }
    public:
        bool is_valid_tree() const
        {
            node_handle_t rootNode = get_node(m_rootIndex);
            if (!rootNode.is_valid())
            {
                // an empty tree is still a valid tree
                return true;
            }

            if (!rootNode.is_black())
            {
                // error: the root node must always be black
                return false;
            }

            return is_valid_subtree(rootNode);
        }
#pragma endregion

    protected:
        node_handle_t allocate_node(T value)
        {
            const size_t nodeIndex = m_allocator.AllocateBlock();
            m_allocator[nodeIndex] = node_t(value, NIL, NIL, NIL, rbtreecolors::RED);
            m_size += 1;
            return get_node(nodeIndex);
        }

    private:

        struct node_t
        {
            T value;
            size_t parent;
            size_t left;
            size_t right;
            rbtreecolors color;

            node_t() = default;
            node_t(T inValue, size_t inParent, rbtreecolors inColor = rbtreecolors::RED)
                : value{inValue}, parent{inParent}, color{inColor}
            {}
            node_t(T inValue, size_t inParent, size_t inLeft, size_t inRight, rbtreecolors inColor)
                : value{inValue}, parent{inParent}, left{inLeft}, right{inRight}, color{inColor}
            {}
        };

        struct node_handle_t
        {
            size_t index = NIL;
            const Allocator* allocator = nullptr;

            node_handle_t() = default;
            node_handle_t(size_t inIndex, const Allocator* inAllocator)
                : index{inIndex}, allocator{inAllocator}
            {}

            const static node_handle_t NilNode;

            inline bool is_valid() const { return index != NIL && allocator != nullptr; }

            inline node_t& node() const 
            { 
                return (*allocator)[index]; 
            }

            inline node_handle_t parent() const 
            { 
                return is_valid() && node().parent != NIL? node_handle_t(node().parent, allocator) : NilNode; 
            }

            inline node_handle_t left() const 
            { 
                return is_valid()? node_handle_t(node().left, allocator) : NilNode; 
            }

            inline node_handle_t right() const 
            { 
                return is_valid()? node_handle_t(node().right, allocator) : NilNode; 
            }

            inline node_handle_t grand_parent() const 
            { 
                return parent().parent(); 
            }

            inline bool is_red() const { return is_valid()? node().color == rbtreecolors::RED : false; }
            inline bool is_black() const { return is_valid()? node().color == rbtreecolors::BLACK : true; }
            inline bool is_leaf() const { return !left().is_valid() && !right().is_valid(); }

            inline void set_parent(node_handle_t newParent) 
            { 
                if (is_valid())
                {
                    node().parent = newParent.index;
                } 
            }

            inline void set_left(node_handle_t newChild) 
            { 
                if (is_valid())
                {
                    node().left = newChild.index; 
                }
            }

            inline void set_right(node_handle_t newChild) 
            { 
                if (is_valid())
                {
                    node().right = newChild.index; 
                }
            }

            inline void set_color(rbtreecolors color) 
            { 
                if (is_valid())
                {
                    node().color = color; 
                }
            }

            inline bool operator==(const node_handle_t& other) const noexcept
            {
                return index == other.index;
            }

            inline bool operator!=(const node_handle_t& other) const noexcept
            {
                return index != other.index;
            }
            
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

                return node_handle_t(NIL, nullptr);
            }

            inline node_handle_t uncle() const { return parent().sibling(); }

            inline bool is_right_child() const 
            { 
                return parent().right().index == index; 
            }

            inline bool is_left_child() const 
            { 
                return parent().left().index == index; 
            }
        };

        size_t m_size = 0;
        size_t m_rootIndex = 0;
        
        Allocator m_allocator;
    };
}

template<typename T, size_t Capacity>
const ecs::rbtree<T, Capacity>::node_handle_t ecs::rbtree<T, Capacity>::node_handle_t::NilNode =
    ecs::rbtree<T, Capacity>::node_handle_t();