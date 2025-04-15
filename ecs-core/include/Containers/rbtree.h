#include <iostream>
#include <stack>
#include <queue>
#include <cassert>
#include <optional>
#include <string>
#include <sstream>

#include "GraphNode.h"
#include "SingleBlockAllocatorTraits.h"
#include "SingleBlockChunkAllocator.h"

namespace ecs
{
    template <typename T, template<typename> class TAllocator = SingleBlockChunkAllocator>
    class rbtree 
    {
    private:
        enum Color { RED, BLACK };

        /**
         * A node of this RB-tree.
         */
        struct rbnode_t : public graph_node_t<T, 2>
        {
            using Base = graph_node_t<T, 2>;

            Color color;

            rbnode_t() : Base(), color(Color::BLACK) {}
            rbnode_t(T inValue, size_t inParent, size_t inLeft, size_t inRight, Color inColor)
                : Base(inValue, inParent, {inLeft, inRight}), color(inColor)
            {}
        };

        using NodeAllocator = TAllocator<rbnode_t>;

        /**
         * Handle for graph node of this RB-tree. Allows utility getters and setters
         * for any field of the node.
         */
        struct rbnode_handle_t 
        {
        public:
            rbnode_handle_t() : m_index{std::nullopt}, m_allocator{nullptr} {}
            rbnode_handle_t(std::optional<size_t> inIndex, NodeAllocator* inAllocator)
                : m_index{inIndex}, m_allocator{inAllocator} {}
            rbnode_handle_t(size_t inIndex, NodeAllocator* inAllocator)
                : m_index{inIndex}, m_allocator{inAllocator} {}

            inline std::optional<size_t> index() const noexcept  { return m_index; }
            inline NodeAllocator* allocator() const noexcept { return m_allocator; }

            inline rbnode_t& node() const 
            {
                assert(m_allocator != nullptr);
                assert(m_index.has_value());
                // @todo assert index is valid
                return (*m_allocator)[*m_index];
            }

            inline bool is_nil() const noexcept { return !m_index.has_value(); }

            inline T value() const 
            {
                return node().value;
            }
    
            inline rbnode_handle_t parent() const 
            {
                return rbnode_handle_t(node().parent, m_allocator);
            }

            inline rbnode_handle_t left() const 
            {
                return rbnode_handle_t(node().children[0], m_allocator);
            }

            inline rbnode_handle_t right() const 
            {
                return rbnode_handle_t(node().children[1], m_allocator);
            }

            inline Color color() const noexcept 
            {
                return node().color;
            }
    
            inline void set_value(T value)
            {
                node().value = value;
            }
    
            inline void set_parent(rbnode_handle_t newParent) 
            {
                node().parent = newParent.index();
            }
    
            inline void set_left(rbnode_handle_t newChild) 
            {
                node().children[0] = newChild.index();
            }

            inline void set_right(rbnode_handle_t newChild) 
            {
                node().children[1] = newChild.index();
            }

            inline void set_color(Color newColor)
            {
                node().color = newColor;
            }
    
            bool operator==(const rbnode_handle_t& other) const 
            {
                if (is_nil() && other.is_nil())
                {
                    return true;
                }

                return m_index == other.index() && m_allocator == other.m_allocator;
            }
    
            bool operator!=(const rbnode_handle_t& other) const 
            {
                return !(*this == other);
            }

        private:
            std::optional<size_t> m_index;
            NodeAllocator* m_allocator;
        };

        using NodeHandle = rbnode_handle_t;

        NodeHandle m_root;
        size_t m_size;
        NodeAllocator m_allocator;
        static const NodeHandle NIL;

    public:
        rbtree() : m_size(0)
        {
            m_root = rbnode_handle_t(std::nullopt, &m_allocator);
        }
    
        ~rbtree() 
        {
            clear_recursive(m_root);
        }

        rbtree(const rbtree& other)
        {
            m_size = other.size();
            copy_recursive(other.m_root, NIL);
        }

        rbtree(rbtree&& other)
        {
            m_allocator = other.m_allocator;
            m_root = other.m_root;
            m_size = other.m_size;
        }

        rbtree(std::initializer_list<T> initializerList) : rbtree()
        {
            for (const T& element : initializerList)
            {
                insert(element);
            }
        }

        rbtree& operator=(const rbtree& other)
        {
            m_size = other.size();
            copy_recursive(other.m_root, NIL);
            return *this;
        }

        rbtree& operator=(rbtree& other)
        {
            m_allocator = other.m_allocator;
            m_root = other.m_root;
            m_size = other.m_size;
        }

        bool operator==(const rbtree& other) const 
        {
            return equals(other);
        }

        bool operator!=(const rbtree& other) const 
        {
            return !equals(other);
        }

    
        // Returns true if the value was inserted, false if it already exists
        bool insert(const T& value) 
        {
            // Check if value already exists
            if (!search(value).is_nil()) 
            {
                return false;
            }
            
            NodeHandle newNode = allocate_node();
            newNode.set_value(value);
            newNode.set_color(RED);
            NodeHandle y = NIL;
            NodeHandle x = m_root;
            
            while (x != NIL) 
            {
                y = x;
                if (value < x.value()) 
                {
                    x = x.left();
                } 
                else if (value > x.value())
                {
                    x = x.right();
                }
                else
                {
                    deallocate_node(newNode);
                    return false;
                }
            }
            
            newNode.set_parent(y);
            if (y == NIL) 
            {
                m_root = newNode;
            } 
            else if (value < y.value()) 
            {
                y.set_left(newNode);
            } 
            else 
            {
                y.set_right(newNode);
            }
            
            // Fix the tree
            if (newNode.parent() == NIL) 
            {
                newNode.set_color(BLACK);
                m_size++;
                return true;
            }
            
            if (newNode.parent().parent() == NIL) 
            {
                m_size++;
                return true;
            }
            
            fix_insert(newNode);
            m_size++;
            return true;
        }
    
        // Returns true if the value was erased, false if it wasn't found
        bool erase(const T& value) 
        {
            NodeHandle z = search(value);
            if (z == NIL) 
            {
                return false;
            }
            
            NodeHandle y = z;
            NodeHandle x = NIL;
            NodeHandle xParent = NIL;
            bool x_is_left_child = false;
            Color y_original_color = y.color();
            
            if (z.left() == NIL) 
            {
                x = z.right();
                xParent = z.parent();
                x_is_left_child = (z.parent() != NIL && z == z.parent().left());
                transplant(z, z.right());
            } 
            else if (z.right() == NIL) 
            {
                x = z.left();
                xParent = z.parent();
                x_is_left_child = (z.parent() != NIL && z == z.parent().left());
                transplant(z, z.left());
            } 
            else 
            {
                y = minimum(z.right());
                y_original_color = y.color();
                x = y.right();
                
                if (y.parent() == z) 
                {
                    if (x != NIL) 
                    {
                        x.set_parent(y);
                    }
                    xParent = y;
                    x_is_left_child = false; // x is a right child of y
                } 
                else 
                {
                    xParent = y.parent();
                    x_is_left_child = (y == y.parent().left());
                    transplant(y, y.right());
                    y.set_right(z.right());
                    if (y.right() != NIL) 
                    {
                        y.right().set_parent(y);
                    }
                }
                
                transplant(z, y);
                y.set_left(z.left());
                if (y.left() != NIL) 
                {
                    y.left().set_parent(y);
                }
                y.set_color(z.color());
            }
            
            if (y_original_color == BLACK) 
            {
                fix_erase(x, xParent, x_is_left_child);
            }
            
            deallocate_node(z);
            m_size--;
            return true;
        }
    
        // Verifies that the tree satisfies all red-black tree properties
        bool is_valid_tree() const 
        {
            if (m_root == NIL) 
            {
                return true;
            }
            
            // Property 1: Every node is either red or black. (Enforced by enum)
            
            // Property 2: The root is black
            if (m_root.color() != BLACK) 
            {
                return false;
            }
            
            // Property 3: Every leaf (NULL) is black. (Implicitly true)
            
            // Property 4: If a node is red, then both its children are black.
            if (!check_red_property(m_root)) 
            {
                return false;
            }
            
            // Property 5: For each node, all simple paths from the node to descendant leaves 
            // contain the same number of black nodes.
            int height = 0;
            if (!check_black_height(m_root, height)) 
            {
                return false;
            }
            
            // Additionally, check that it's a valid binary search tree
            std::optional<T> minValue = std::nullopt;
            std::optional<T> maxValue = std::nullopt;
            return is_binary_search_tree(m_root, minValue, maxValue);
        }

        // Additional utility functions
        size_t size() const 
        {
            return m_size;
        }
    
        bool contains(const T& value) const 
        {
            return search(value) != NIL;
        }
    
        bool empty() const 
        {
            return m_root == NIL;
        }
    
        void clear() 
        {
            clear_recursive(m_root);
            m_root = NIL;
            m_size = 0;
        }

        std::string to_string() const 
        {
            std::stringstream ss;
            ss << "RedBlackTree (size=" << m_size << "):" << std::endl;
            
            if (m_root == NIL) 
            {
                ss << "    [Empty tree]" << std::endl;
                return ss.str();
            }
            
            // Use recursive helper to build the tree representation
            to_string_recursive(m_root, ss);
            
            return ss.str();
        }

        struct iterator
        {
        public:
            iterator()
            {
                m_current = rbtree<T, TAllocator>::NIL;
            }

            iterator(const rbtree<T, TAllocator>& tree)
            {
                m_current = tree.minimum(tree.get_root());
            };

            iterator(NodeHandle node) : m_current{node}
            {}

            inline iterator& operator++() 
            {
                if (m_current.is_nil())
                {
                    throw std::out_of_range("Reached the end of the iterator");
                }

                bool mustGoUpwards = m_current.right().is_nil();
                if (mustGoUpwards)
                {
                    // go up until we either find a NIL node or we find a node whose value is higher than current.
                    const T currentValue = m_current.value();
                    while (!m_current.is_nil() && m_current.value() <= currentValue)
                    {
                        m_current = m_current.parent();
                    }
                }
                else
                {
                    m_current = m_current.right();
                    if (m_current.is_nil())
                    {
                        // reached the end
                        return *this;
                    }

                    // go down until we find a valid node
                    while (!m_current.left().is_nil())
                    {
                        m_current = m_current.left();
                    }
                }

                return *this;
            }

            inline T value() const noexcept 
            {
                return m_current.value();
            }

            bool operator==(const iterator& other) const 
            {
                return m_current == other.m_current;
            }

            explicit operator bool() const 
            {
                return !m_current.is_nil();
            }

            const T& operator*() const 
            {
                return m_current.node().value;
            }

        private:
            NodeHandle m_current;
        };

        inline iterator begin() const noexcept 
        {
            return iterator(*this);
        }

        inline iterator end() const noexcept 
        {
            return iterator();
        }

        inline iterator find(T value) const noexcept 
        {
            return iterator(search(value));
        }

    private:
        

        NodeHandle allocate_node()
        {
            const size_t allocatedIndex = SingleBlockAllocatorTrait<NodeAllocator, rbnode_t>::AllocateBlock(m_allocator);
            return NodeHandle(allocatedIndex, &m_allocator);
        }

        void deallocate_node(NodeHandle nodeHandle)
        {
            if (nodeHandle != NIL)
            {
                SingleBlockAllocatorTrait<NodeAllocator, rbnode_t>::FreeBlock(m_allocator, nodeHandle.index().value());
            }
        }

        NodeHandle get_root() const 
        {
            return m_root;
        }
    
        // Helper methods
        void rotate_left(NodeHandle x) 
        {
            NodeHandle y = x.right();
            x.set_right(y.left());
            
            if (y.left() != NIL) 
            {
                y.left().set_parent(x);
            }
            
            y.set_parent(x.parent());
            
            if (x.parent() == NIL) 
            {
                m_root = y;
            } 
            else if (x == x.parent().left())
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
    
        void rotate_right(NodeHandle x) 
        {
            NodeHandle y = x.left();
            x.set_left(y.right());
            
            if (y.right() != NIL) 
            {
                y.right().set_parent(x);
            }
            
            y.set_parent(x.parent());
            
            if (x.parent() == NIL) 
            {
                m_root = y;
            } 
            else if (x == x.parent().left()) 
            {
                x.parent().set_left(y);
            } 
            else 
            {
                x.parent().set_right(y);
            }
            
            y.set_right(x);
            x.set_parent(y);
        }
    
        void fix_insert(NodeHandle k) 
        {
            NodeHandle u;
            
            while (k.parent() != NIL && k.parent().color() == RED) 
            {
                if (k.parent() == k.parent().parent().right()) 
                {
                    u = k.parent().parent().left();
                    if (u != NIL && u.color() == RED) 
                    {
                        u.set_color(BLACK);
                        k.parent().set_color(BLACK);
                        k.parent().parent().set_color(RED);
                        k = k.parent().parent();
                    }
                    else 
                    {
                        if (k == k.parent().left()) 
                        {
                            k = k.parent();
                            rotate_right(k);
                        }
                        k.parent().set_color(BLACK);
                        k.parent().parent().set_color(RED);
                        rotate_left(k.parent().parent());
                    }
                } 
                else 
                {
                    u = k.parent().parent().right();
                    if (u != NIL && u.color() == RED) 
                    {
                        u.set_color(BLACK);
                        k.parent().set_color(BLACK);
                        k.parent().parent().set_color(RED);
                        k = k.parent().parent();
                    } 
                    else 
                    {
                        if (k == k.parent().right()) 
                        {
                            k = k.parent();
                            rotate_left(k);
                        }
                        k.parent().set_color(BLACK);
                        k.parent().parent().set_color(RED);
                        rotate_right(k.parent().parent());
                    }
                }
                if (k == m_root) 
                {
                    break;
                }
            }
            m_root.set_color(BLACK);
        }
    
        NodeHandle search(const T& value) const 
        {
            NodeHandle current = m_root;
            while (current != NIL) 
            {
                if (value == current.value()) 
                {
                    return current;
                } 
                else if (value < current.value()) 
                {
                    current = current.left();
                } 
                else 
                {
                    current = current.right();
                }
            }
            return NIL;
        }
    
        NodeHandle minimum(NodeHandle node) const 
        {
            if (node == NIL) 
            {
                return NIL;
            }
            
            while (node.left() != NIL) 
            {
                node = node.left();
            }
            
            return node;
        }
    
        void transplant(NodeHandle u, NodeHandle v) 
        {
            if (u.parent() == NIL) 
            {
                m_root = v;
            } 
            else if (u == u.parent().left()) 
            {
                u.parent().set_left(v);
            } 
            else 
            {
                u.parent().set_right(v);
            }
            
            if (v != NIL) 
            {
                v.set_parent(u.parent());
            }
        }
    
        void fix_erase(NodeHandle x, NodeHandle xParent, bool xIsLeftChild) 
        {
            NodeHandle w;
            
            while ((x == NIL || x.color() == BLACK) && x != m_root) 
            {
                if (x == NIL) 
                {
                    if (xIsLeftChild) 
                    {
                        w = xParent.right();
                        if (w.color() == RED) 
                        {
                            w.set_color(BLACK);
                            xParent.set_color(RED);
                            rotate_left(xParent);
                            w = xParent.right();
                        }
                        
                        if ((w.left() == NIL || w.left().color() == BLACK) && 
                            (w.right() == NIL || w.right().color() == BLACK)) 
                        {
                            w.set_color(RED);
                            x = xParent;
                            if (x.parent() != NIL) 
                            {
                                xIsLeftChild = (x == x.parent().left());
                                xParent = x.parent();
                            }
                        } 
                        else 
                        {
                            if (w.right() == NIL || w.right().color() == BLACK) 
                            {
                                if (w.left() != NIL) 
                                {
                                    w.left().set_color(BLACK);
                                }
                                w.set_color(RED);
                                rotate_right(w);
                                w = xParent.right();
                            }
                            
                            w.set_color(xParent.color());
                            xParent.set_color(BLACK);
                            if (w.right() != NIL) 
                            {
                                w.right().set_color(BLACK);
                            }
                            rotate_left(xParent);
                            x = m_root;
                        }
                    } 
                    else 
                    {   
                        // x is a right child
                        w = xParent.left();
                        if (w.color() == RED) 
                        {
                            w.set_color(BLACK);
                            xParent.set_color(RED);
                            rotate_right(xParent);
                            w = xParent.left();
                        }
                        
                        if ((w.right() == NIL || w.right().color() == BLACK) && 
                            (w.left() == NIL || w.left().color() == BLACK)) 
                        {
                            w.set_color(RED);
                            x = xParent;
                            if (x.parent() != NIL) 
                            {
                                xIsLeftChild = (x == x.parent().left());
                                xParent = x.parent();
                            }
                        } 
                        else 
                        {
                            if (w.left() == NIL || w.left().color() == BLACK) 
                            {
                                if (w.right() != NIL) 
                                {
                                    w.right().set_color(BLACK);
                                }
                                w.set_color(RED);
                                rotate_left(w);
                                w = xParent.left();
                            }
                            
                            w.set_color(xParent.color());
                            xParent.set_color(BLACK);
                            if (w.left() != NIL) 
                            {
                                w.left().set_color(BLACK);
                            }
                            rotate_right(xParent);
                            x = m_root;
                        }
                    }
                } 
                else 
                { 
                    // x is not nullptr
                    if (x == x.parent().left()) 
                    {
                        w = x.parent().right();
                        if (w.color() == RED) 
                        {
                            w.set_color(BLACK);
                            x.parent().set_color(RED);
                            rotate_left(x.parent());
                            w = x.parent().right();
                        }
                        
                        if ((w.left() == NIL || w.left().color() == BLACK) && 
                            (w.right() == NIL || w.right().color() == BLACK)) 
                        {
                            w.set_color(RED);
                            x = x.parent();
                        } 
                        else 
                        {
                            if (w.right() == NIL || w.right().color() == BLACK) 
                            {
                                if (w.left() != NIL) 
                                {
                                    w.left().set_color(BLACK);
                                }
                                w.set_color(RED);
                                rotate_right(w);
                                w = x.parent().right();
                            }
                            
                            w.set_color(x.parent().color());
                            x.parent().set_color(BLACK);
                            if (w.right() != NIL) 
                            {
                                w.right().set_color(BLACK);
                            }
                            rotate_left(x.parent());
                            x = m_root;
                        }
                    } 
                    else 
                    { // x is a right child
                        w = x.parent().left();
                        if (w.color() == RED) 
                        {
                            w.set_color(BLACK);
                            x.parent().set_color(RED);
                            rotate_right(x.parent());
                            w = x.parent().left();
                        }
                        
                        if ((w.right() == NIL || w.right().color() == BLACK) && 
                            (w.left() == NIL || w.left().color() == BLACK)) 
                        {
                            w.set_color(RED);
                            x = x.parent();
                        } 
                        else 
                        {
                            if (w.left() == NIL || w.left().color() == BLACK) 
                            {
                                if (w.right() != NIL) 
                                {
                                    w.right().set_color(BLACK);
                                }
                                w.set_color(RED);
                                rotate_left(w);
                                w = x.parent().left();
                            }
                            
                            w.set_color(x.parent().color());
                            x.parent().set_color(BLACK);
                            if (w.left() != NIL) 
                            {
                                w.left().set_color(BLACK);
                            }
                            rotate_right(x.parent());
                            x = m_root;
                        }
                    }

                    xParent = x.parent();
                    if (xParent != NIL) 
                    {
                        xIsLeftChild = (x == xParent.left());
                    }
                }
            }
            
            if (x != NIL) 
            {
                x.set_color(BLACK);
            }
        }
    
        void clear_recursive(NodeHandle node) 
        {
            if (node == NIL) 
            {
                return;
            }
            
            clear_recursive(node.left());
            clear_recursive(node.right());
            deallocate_node(node);
        }
    
        // Validation methods
        bool is_binary_search_tree(NodeHandle node, std::optional<T>& min_value, std::optional<T>& max_value) const 
        {
            if (node == NIL) 
            {
                return true;
            }
            
            if ((min_value.has_value() && node.value() <= min_value) || 
                (max_value.has_value() && node.value() >= max_value)) 
            {
                return false;
            }
            
            std::optional<T> nodeValue = node.value(); 
            return is_binary_search_tree(node.left(), min_value, nodeValue) && 
                   is_binary_search_tree(node.right(), nodeValue, max_value);
        }
    
        bool check_red_property(NodeHandle node) const 
        {
            if (node == NIL) {
                return true;
            }
            
            if (node.color() == RED) {
                if ((node.left() != NIL && node.left().color() == RED) || 
                    (node.right() != NIL && node.right().color() == RED)) {
                    return false;
                }
            }
            
            return check_red_property(node.left()) && check_red_property(node.right());
        }
    
        bool check_black_height(NodeHandle node, int& height) const 
        {
            if (node == NIL) 
            {
                height = 1; // Null nodes are considered BLACK
                return true;
            }
            
            int left_height = 0, right_height = 0;
            if (!check_black_height(node.left(), left_height) || 
                !check_black_height(node.right(), right_height)) 
            {
                return false;
            }
            
            if (left_height != right_height) 
            {
                return false;
            }
            
            height = left_height + (node.color() == BLACK ? 1 : 0);
            return true;
        }

        void to_string_recursive(NodeHandle node, std::stringstream& ss, int depth = 0) const 
        {
            if (node == NIL) 
            {
                ss << std::string(depth * 4, ' ') << "NULL" << std::endl;
                return;
            }
            
            ss << std::string(depth * 4, ' ') << node.value() << " (" 
               << (node.color() == RED ? "RED" : "BLACK") << ")" << std::endl;
            
            to_string_recursive(node.left(), ss, depth + 1);
            to_string_recursive(node.right(), ss, depth + 1);
        }

        NodeHandle copy_recursive(NodeHandle nodeToCopy, NodeHandle parentOfCopy)
        {
            if (nodeToCopy.is_nil())
            {
                return NIL;
            }

            NodeHandle myNode = allocate_node();
            NodeHandle left = copy_recursive(nodeToCopy.left(), myNode);
            NodeHandle right = copy_recursive(nodeToCopy.right(), myNode);

            myNode.set_value(nodeToCopy.value());
            myNode.set_left(left);
            myNode.set_right(right);
            myNode.set_color(nodeToCopy.color());
            myNode.set_parent(parentOfCopy);
            if (parentOfCopy.is_nil())
            {
                m_root = myNode;
            }

            return myNode;
        }

        bool equals(const rbtree& other) const 
        {
            if (m_size != other.size())
            {
                return false;
            }

            auto myIt = begin();
            auto otherIt = other.begin();
            while (myIt != end())
            {
                if (*myIt != *otherIt)
                {
                    return false;
                }

                ++myIt;
                ++otherIt;
            }

            return true;
        }
    };

    template <typename T, template<typename> class TAllocator>
    const rbtree<T, TAllocator>::NodeHandle rbtree<T, TAllocator>::NIL = rbtree<T, TAllocator>::rbnode_handle_t(std::nullopt, nullptr);

    template <typename T, template<typename> class TAllocator>
    rbtree<T, TAllocator> make_intersection(const rbtree<T, TAllocator>& tree1, const rbtree<T, TAllocator>& tree2)
    {
        auto i1 = tree1.begin();
        auto i2 = tree2.begin();

        rbtree<T, TAllocator> intersection;
        while (i1 != tree1.end() && i2 != tree2.end())
        {
            const T v1 = *i1;
            const T v2 = *i2;
            if (v1 == v2)
            {
                ++i1;
                ++i2;
                intersection.insert(v1);
            }
            else if (v1 > v2)
            {
                ++i2;
            }
            else 
            {
                ++i1;
            }
        }

        return intersection;
    }
}
