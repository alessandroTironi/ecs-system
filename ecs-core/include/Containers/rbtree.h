#pragma once 

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <limits>
#include <string>
#include <format>
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
                set_root(newNode);
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
            // Find the node to delete
            node_handle_t z = find_node(value);
            
            if (!z.is_valid())
            {
                // Node not found
                return;
            }

            node_handle_t y = z;
            node_handle_t x;
            rbtreecolors yOriginalColor = y.node().color;

            if (!z.left().is_valid())
            {
                x = z.right();
                transplant(z, z.right());
            }
            else if (!z.right().is_valid())
            {
                x = z.left();
                transplant(z, z.left());
            }
            else
            {
                // Find successor (minimum in right subtree)
                y = find_minimum(z.right());
                yOriginalColor = y.node().color;
                x = y.right();

                if (y.parent().index == z.index)
                {
                    // y is direct child of z
                    if (x.is_valid())
                    {
                        x.set_parent(y);
                    }
                }
                else
                {
                    // y is further down in the tree
                    transplant(y, y.right());
                    y.set_right(z.right());
                    if (y.right().is_valid())
                    {
                        y.right().set_parent(y);
                    }
                }

                transplant(z, y);
                y.set_left(z.left());
                if (y.left().is_valid())
                {
                    y.left().set_parent(y);
                }
                y.set_color(z.node().color);
            }

            // Only fix the tree if we removed a black node
            if (yOriginalColor == rbtreecolors::BLACK)
            {
                fixup_erase(x);
            }

            // Deallocate the removed node
            deallocate_node(z);
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

#ifdef DEBUG_BUILD
        bool enableDebugMode = false;
#endif

        std::string to_string(node_handle_t rootNode, size_t depth) const
        {
            std::string thisString;
            for (int i = 0; i < depth; ++i)
                thisString = std::format("  {}", thisString);
            thisString = std::format("{}{}{}", thisString,
                m_rootIndex == rootNode.index? "": 
                (
                    rootNode.is_right_child()? "R:": "L:"
                ),
                rootNode.to_string());

            if (rootNode.is_valid())
            {
                return std::format("{}:\n{}\n{}", thisString, 
                    to_string(rootNode.right(), depth + 1),
                    to_string(rootNode.left(), depth + 1));
            }

            return "";
        }

        std::string to_string() const 
        {
            return to_string(get_node(m_rootIndex), 0);
        }

    private:
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

        node_handle_t find_node(T value) const
        {
            node_handle_t current = get_node(m_rootIndex);
            
            while (current.is_valid())
            {
                if (value < current.node().value)
                {
                    current = current.left();
                }
                else if (value > current.node().value)
                {
                    current = current.right();
                }
                else
                {
                    // Found the node
                    return current;
                }
            }
            
            // Node not found
            return node_handle_t::NilNode;
        }

        node_handle_t get_node(size_t index) const
        {
            if (index == NIL)
            {
                return node_handle_t::NilNode;
            }

            return node_handle_t(index, &m_allocator, this);
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
                set_root(y);
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
                set_root(y);
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
                set_root(v);
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
            while (x.index != m_rootIndex && (x.is_valid() && x.is_black()))
            {
                if (x.is_left_child())
                {
                    node_handle_t w = x.parent().right();
                    
                    // Case 1: x's sibling w is red
                    if (w.is_red())
                    {
                        w.set_color(rbtreecolors::BLACK);
                        x.parent().set_color(rbtreecolors::RED);
                        left_rotate(x.parent());
                        w = x.parent().right();
                    }

                    // Case 2: x's sibling w is black, and both of w's children are black
                    if ((w.left().is_valid() ? w.left().is_black() : true) && 
                        (w.right().is_valid() ? w.right().is_black() : true))
                    {
                        w.set_color(rbtreecolors::RED);
                        x = x.parent();
                    }
                    else
                    {
                        // Case 3: x's sibling w is black, w's left child is red, w's right child is black
                        if (w.right().is_valid() ? w.right().is_black() : true)
                        {
                            if (w.left().is_valid())
                            {
                                w.left().set_color(rbtreecolors::BLACK);
                            }
                            w.set_color(rbtreecolors::RED);
                            right_rotate(w);
                            w = x.parent().right();
                        }
                        
                        // Case 4: x's sibling w is black, and w's right child is red
                        w.set_color(x.parent().node().color);
                        x.parent().set_color(rbtreecolors::BLACK);
                        if (w.right().is_valid())
                        {
                            w.right().set_color(rbtreecolors::BLACK);
                        }
                        left_rotate(x.parent());
                        x = get_node(m_rootIndex); // Set x to root to terminate the loop
                    }
                }
                else // x is a right child
                {
                    node_handle_t w = x.parent().left();
                    
                    // Case 1: x's sibling w is red
                    if (w.is_red())
                    {
                        w.set_color(rbtreecolors::BLACK);
                        x.parent().set_color(rbtreecolors::RED);
                        right_rotate(x.parent());
                        w = x.parent().left();
                    }

                    // Case 2: x's sibling w is black, and both of w's children are black
                    if ((w.right().is_valid() ? w.right().is_black() : true) && 
                        (w.left().is_valid() ? w.left().is_black() : true))
                    {
                        w.set_color(rbtreecolors::RED);
                        x = x.parent();
                    }
                    else
                    {
                        // Case 3: x's sibling w is black, w's right child is red, w's left child is black
                        if (w.left().is_valid() ? w.left().is_black() : true)
                        {
                            if (w.right().is_valid())
                            {
                                w.right().set_color(rbtreecolors::BLACK);
                            }
                            w.set_color(rbtreecolors::RED);
                            left_rotate(w);
                            w = x.parent().left();
                        }
                        
                        // Case 4: x's sibling w is black, and w's left child is red
                        w.set_color(x.parent().node().color);
                        x.parent().set_color(rbtreecolors::BLACK);
                        if (w.left().is_valid())
                        {
                            w.left().set_color(rbtreecolors::BLACK);
                        }
                        right_rotate(x.parent());
                        x = get_node(m_rootIndex); // Set x to root to terminate the loop
                    }
                }
            }

            // Ensure x is black (if it's a valid node)
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
    public:
        bool is_valid_tree() const
        {
            node_handle_t root = get_node(m_rootIndex);
            
            // Empty tree is valid
            if (!root.is_valid())
                return true;
            
            // Rule 2: The root must be black
            if (!root.is_black())
                return false;
            
            // Track the maximum and minimum black height
            int minBlackHeight = std::numeric_limits<int>::max();
            int maxBlackHeight = std::numeric_limits<int>::min();
            
            // Validate all properties using a recursive helper
            bool valid = validate_node(root, minBlackHeight, maxBlackHeight);
            
            // Rule 5: All paths must have the same black height
            return valid && (minBlackHeight == maxBlackHeight);
        }

    private:
        bool validate_node(node_handle_t node, int& minBlackHeight, int& maxBlackHeight) const
        {
            // NIL nodes are black by definition
            if (!node.is_valid())
            {
                minBlackHeight = maxBlackHeight = 0;
                return true;
            }
            
            // Validate left subtree
            int leftMinBlackHeight = std::numeric_limits<int>::max();
            int leftMaxBlackHeight = std::numeric_limits<int>::min();
            bool leftValid = validate_node(node.left(), leftMinBlackHeight, leftMaxBlackHeight);
            if (!leftValid)
                return false;
            
            // Validate right subtree
            int rightMinBlackHeight = std::numeric_limits<int>::max();
            int rightMaxBlackHeight = std::numeric_limits<int>::min();
            bool rightValid = validate_node(node.right(), rightMinBlackHeight, rightMaxBlackHeight);
            if (!rightValid)
                return false;
            
            // Rule 4: Red nodes cannot have red children
            if (node.is_red())
            {
                if ((node.left().is_valid() && node.left().is_red()) || 
                    (node.right().is_valid() && node.right().is_red()))
                    return false;
            }
            
            // Validate parent-child relationships
            if (node.left().is_valid() && node.left().parent().index != node.index)
                return false;
            
            if (node.right().is_valid() && node.right().parent().index != node.index)
                return false;
            
            // Calculate black height for this node
            int blackIncrement = node.is_black() ? 1 : 0;
            minBlackHeight = std::min(leftMinBlackHeight, rightMinBlackHeight) + blackIncrement;
            maxBlackHeight = std::max(leftMaxBlackHeight, rightMaxBlackHeight) + blackIncrement;
            
            // Rule 5: Black height must be the same for all paths in the subtree
            return leftMinBlackHeight == leftMaxBlackHeight && 
                rightMinBlackHeight == rightMaxBlackHeight &&
                leftMinBlackHeight == rightMinBlackHeight;
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

        void deallocate_node(node_handle_t node)
        {
            m_allocator.FreeBlock(node.index);
            m_size -= 1;
        }

        void set_root(node_handle_t newRoot)
        {
            m_rootIndex = newRoot.index;
            newRoot.set_parent(node_handle_t::NilNode);

#ifdef DEBUG_BUILD
            if (enableDebugMode)
            {
                std::cout << "Set root to " << newRoot.to_string() << std::endl;
            }
#endif
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
            const rbtree<T, InitialCapacity>* owner = nullptr;

            node_handle_t() = default;
            node_handle_t(size_t inIndex, const Allocator* inAllocator, const rbtree<T, InitialCapacity>* inOwner)
                : index{inIndex}, allocator{inAllocator}, owner{inOwner}
            {}

            const static node_handle_t NilNode;

            inline bool is_valid() const { return index != NIL && allocator != nullptr; }

            inline node_t& node() const 
            { 
                return (*allocator)[index]; 
            }

            inline node_handle_t parent() const 
            { 
                return is_valid() && node().parent != NIL? node_handle_t(node().parent, allocator, owner) : NilNode; 
            }

            inline node_handle_t left() const 
            { 
                return is_valid()? node_handle_t(node().left, allocator, owner) : NilNode; 
            }

            inline node_handle_t right() const 
            { 
                return is_valid()? node_handle_t(node().right, allocator, owner) : NilNode; 
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

#ifdef DEBUG_BUILD
                if (owner->enableDebugMode)
                {
                    std::cout << "Set " << to_string() << "'s parent to " << newParent.to_string() << std::endl;
                }
#endif
            }

            inline void set_left(node_handle_t newChild) 
            { 
                if (is_valid())
                {
                    node().left = newChild.index; 
                }

#ifdef DEBUG_BUILD
                if (owner->enableDebugMode)
                {
                    std::cout << "Set " << to_string() << "'s left child to " << newChild.to_string() << std::endl;
                }
#endif
            }

            inline void set_right(node_handle_t newChild) 
            { 
                if (is_valid())
                {
                    node().right = newChild.index; 

#ifdef DEBUG_BUILD
                    if (owner->enableDebugMode)
                    {
                        std::cout << "Set " << to_string() << "'s right child to " << newChild.to_string() << std::endl;
                    }
#endif
                }
            }

            inline void set_color(rbtreecolors color) 
            { 
                if (is_valid())
                {
                    node().color = color; 

#ifdef DEBUG_BUILD
                    if (owner->enableDebugMode)
                    {
                        std::cout << "Set color of " << to_string() << " to " << (color == rbtreecolors::BLACK? "BLACK" : "RED") << std::endl;
                    }
#endif
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

            inline std::string to_string() const 
            {
                if (!is_valid())
                {
                    return "INVALID";
                }

                std::string s = std::format("{} ({}{})", node().value, index,
                    (is_red()? "R" : "B"));
                return s;
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