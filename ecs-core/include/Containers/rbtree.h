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
        NIL,
        RED,
        BLACK
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
            node_t newNode(value, NIL, NIL, NIL, rbtreecolors::RED);
            node_handle_t y = node_handle_t(NIL, &m_allocator);
            node_handle_t x = get_node(m_rootIndex);
            while (x != node_handle_t::NilNode)
            {
                y = x;
                if (value < x.node().value)
                {
                    x = x.left();
                }
                else
                {
                    x = x.right();
                }
            }
            newNode.parent = y.index;
            node_handle_t z = node_handle_t::NilNode;

            if (!y.is_valid())
            {
                m_rootIndex = m_allocator.AllocateBlock();
                m_allocator[m_rootIndex] = newNode;
                z = get_node(m_rootIndex);
            }
            else if (newNode.value < y.node().value)
            {
                const size_t zIndex = m_allocator.AllocateBlock();
                m_allocator[zIndex] = newNode;
                z = get_node(zIndex);
                y.set_left_child(z);
            }
            else if (newNode.value > y.node().value)
            {
                const size_t zIndex = m_allocator.AllocateBlock();
                m_allocator[zIndex] = newNode;
                z = get_node(zIndex);
                y.set_right_child(z);
            }
            else
            {
                return;
            }

            m_size += 1;
            z.set_left_child(node_handle_t::NilNode);
            z.set_right_child(node_handle_t::NilNode);
            z.set_color(rbtreecolors::RED);
            balance_tree(z);
        }

        void erase(T value)
        {
            // find node to delete
            node_handle_t toDelete = binary_search(value);
            if (!toDelete.is_valid())
            {
                // node is not in the tree
                return;
            }

            rbtreecolors originalColor = toDelete.node().color;
            node_handle_t x = node_handle_t::NilNode;

            if (!toDelete.left().is_valid())
            {
                // Case 1: left child of the node to delete is NIL: replace the node to
                // delete with its right subtree
                x = toDelete.right();
                toDelete.parent().set_right_child(x);
                x.set_parent(toDelete.parent());
                if (toDelete.index == m_rootIndex)
                {
                    m_rootIndex = x.index;
                }
            }
            else if (!toDelete.right().is_valid())
            {
                // Case 2: right child of the node to delete is NIL: replace the node to
                // delete with its left subtree
                x = toDelete.left();
                toDelete.parent().set_left_child(x);
                x.set_parent(toDelete.parent());
                if (toDelete.index == m_rootIndex)
                {
                    m_rootIndex = x.index;
                }
            }
            else
            {
                // Case 3: no NIL childs
                node_handle_t y = find_minimum(toDelete);
                originalColor = y.node().color;
                x = y.right();
                if (toDelete == y.parent())
                {
                    x.set_parent(y);
                }
                else
                {
                    // transplant y with y's right child
                    transplant_subtree(y, y.right());
                    y.set_right_child(toDelete.right());
                    y.right().set_parent(y);
                }

                transplant_subtree(toDelete, y);
                if (toDelete.left() != y)
                {
                    y.set_left_child(toDelete.left());
                }
                y.left().set_parent(y);
                y.set_color(toDelete.node().color);
                if (toDelete.index == m_rootIndex)
                {
                    m_rootIndex = y.index;
                }
            }

            if (originalColor == rbtreecolors::BLACK)
            {
                fixup_post_erase(x);
            }

            m_size -= 1;
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

        void transplant_subtree(node_handle_t u, node_handle_t v)
        {
            if (!u.parent().is_valid())
            {
                m_rootIndex = v.index;
            }
            else if (u.is_left_child())
            {
                u.parent().set_left_child(v);
            }
            else
            {
                u.parent().set_right_child(v);
            }

            v.set_parent(u.parent());
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
            node_handle_t y = x.right();

            // Turn y's left subtree into x's right subtree
            x.set_right_child(y.left());
            if (y.left().is_valid())
            {
                y.left().set_parent(x); 
            }

            // y's new parent was x's parent
            y.set_parent(x.parent());

            // Check if we need to update the root index
            if (!x.parent().is_valid()) 
            {
                m_rootIndex = y.index;
            }

            // Set the parent to point to y instead of x
            if (x.is_left_child())
            {
                x.parent().set_left_child(y);
            }
            else
            {
                x.parent().set_right_child(y);
            }

            // finally set x as y's left child
            y.set_left_child(x);
            x.set_parent(y);
        }

        void right_rotate(node_handle_t x)
        {
            node_handle_t y = x.left();

            // Turn y's right subtree into x's left subtree
            x.set_left_child(y.right());
            if (y.right().is_valid())
            {
                y.right().set_parent(x); 
            }

            // y's new parent was x's parent
            y.set_parent(x.parent());

            // Check if we need to update the root index
            if (!x.parent().is_valid()) 
            {
                m_rootIndex = y.index;
            }

            // Set the parent to point to y instead of x
            if (x.is_left_child())
            {
                x.parent().set_left_child(y);
            }
            else
            {
                x.parent().set_right_child(y);
            }

            // finally set x as y's right child
            y.set_right_child(x);
            x.set_parent(y);
        }

        void balance_tree(node_handle_t current)
        {
            if (!current.parent().is_valid())
            {
                current.set_color(rbtreecolors::BLACK);
                return;
            }

            // If parent is black, we're done
            if (!current.parent().is_red())
            {
                return;
            }

            while (current.parent().is_valid() && current.parent().is_red())
            {
                if (current.parent().is_left_child())
                {
                    node_handle_t y = current.parent().parent().right();
                    if (y.is_valid() && y.is_red())
                    {
                        current.parent().set_color(rbtreecolors::BLACK);
                        y.set_color(rbtreecolors::BLACK);
                        current.grand_parent().set_color(rbtreecolors::RED);
                        current = current.grand_parent();
                    }
                    else if (current.is_right_child())
                    {
                        current = current.parent();
                        left_rotate(current);
                    }

                    current.parent().set_color(rbtreecolors::BLACK);
                    current.grand_parent().set_color(rbtreecolors::RED);
                    right_rotate(current.grand_parent());
                }
                else if (current.parent().is_right_child())
                {
                    node_handle_t y = current.parent().parent().left();
                    if (y.is_valid() && y.is_red())
                    {
                        current.parent().set_color(rbtreecolors::BLACK);
                        y.set_color(rbtreecolors::BLACK);
                        current.grand_parent().set_color(rbtreecolors::RED);
                        current = current.grand_parent();
                    }
                    else if (current.is_left_child())
                    {
                        current = current.parent();
                        right_rotate(current);
                    }

                    current.parent().set_color(rbtreecolors::BLACK);
                    current.grand_parent().set_color(rbtreecolors::RED);
                    left_rotate(current.grand_parent());
                }
            }

            get_node(m_rootIndex).set_color(rbtreecolors::BLACK);
        }

        void fixup_post_erase(node_handle_t x)
        {
            node_handle_t s = node_handle_t::NilNode;
            while (x.index != m_rootIndex && x.is_black())
            {
                if (x.is_left_child())
                {
                    s = x.parent().right();
                    if (s.is_red())
                    {
                        s.set_color(rbtreecolors::BLACK);
                        x.parent().set_color(rbtreecolors::RED);
                        left_rotate(x.parent());
                        s = x.parent().right();
                    }

                    if (s.left().is_black() && s.right().is_black())
                    {
                        s.set_color(rbtreecolors::RED);
                        x = x.parent();
                    }
                    else
                    {
                        if (s.right().is_black())
                        {
                            s.left().set_color(rbtreecolors::BLACK);
                            s.set_color(rbtreecolors::RED);
                            right_rotate(s);
                            s = x.parent().right();
                        }

                        s.set_color(x.parent().node().color);
                        x.parent().set_color(rbtreecolors::BLACK);
                        s.right().set_color(rbtreecolors::BLACK);
                        left_rotate(x.parent());
                        x = get_node(m_rootIndex);
                    }
                }
                else
                {
                    s = x.parent().left();
                    if (s.is_red())
                    {
                        s.set_color(rbtreecolors::BLACK);
                        x.parent().set_color(rbtreecolors::RED);
                        right_rotate(x.parent());
                        s = x.parent().left();
                    }

                    if (s.left().is_black() && s.right().is_black())
                    {
                        s.set_color(rbtreecolors::RED);
                        x = x.parent();
                    }
                    else
                    {
                        if (s.left().is_black())
                        {
                            s.right().set_color(rbtreecolors::BLACK);
                            s.set_color(rbtreecolors::RED);
                            left_rotate(s);
                            s = x.parent().left();
                        }

                        s.set_color(x.parent().node().color);
                        x.parent().set_color(rbtreecolors::BLACK);
                        s.left().set_color(rbtreecolors::BLACK);
                        right_rotate(x.parent());
                        x = get_node(m_rootIndex);
                    }
                }
            }

            x.set_color(rbtreecolors::BLACK);
        }
        
    private:
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
            if (from.node().value == to.node().value)
            {
                return from.is_black()? 1 : 0;
            }
            else if (to.node().value > from.node().value)
            {
                return count_black_nodes(from.right(), to);
            }   
            else
            {
                return count_black_nodes(from.left(), to);
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
                if (count_black_nodes(rootNode, leafNodes[i]) != requiredNumBlackNodes)
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
            size_t index = NIL;
            const Allocator* allocator = nullptr;

            node_handle_t() = default;
            node_handle_t(size_t inIndex, const Allocator* inAllocator)
                : index{inIndex}, allocator{inAllocator}
            {}

            const static node_handle_t NilNode;

            bool is_valid() const { return index != NIL && allocator != nullptr; }

            inline node_t& node() const 
            { 
                return (*allocator)[index]; 
            }

            inline node_handle_t parent() const { return is_valid() && node().parent != NIL? node_handle_t(node().parent, allocator) : NilNode; }
            inline node_handle_t left() const { return is_valid()? node_handle_t(node().left, allocator) : NilNode; }
            inline node_handle_t right() const { return is_valid()? node_handle_t(node().right, allocator) : NilNode; }
            inline node_handle_t grand_parent() const { return parent().parent(); }
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

            inline void set_left_child(node_handle_t newChild) 
            { 
                if (is_valid())
                {
                    node().left = newChild.index; 
                }
            }

            inline void set_right_child(node_handle_t newChild) 
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

            inline bool is_right_child() const { return parent().right().index == index; }
            inline bool is_left_child() const { return parent().left().index == index; }
        };

        size_t m_size = 0;
        size_t m_rootIndex = 0;
        
        Allocator m_allocator;
    };
}

template<typename T, size_t Capacity>
const ecs::rbtree<T, Capacity>::node_handle_t ecs::rbtree<T, Capacity>::node_handle_t::NilNode =
    ecs::rbtree<T, Capacity>::node_handle_t();