#include <iostream>
#include <stack>
#include <queue>
#include <cassert>

#include "GraphNode.h"
#include "SingleBlockAllocatorTraits.h"
#include "SingleBlockChunkAllocator.h"

namespace ecs
{
    template <typename T, typename TAllocator = SingleBlockChunkAllocator<T>>
    class rbtree 
    {
    public:
        rbtree() : root(nullptr), node_count(0) {}
    
        ~rbtree() 
        {
            clear_recursive(root);
        }
    
        // Returns true if the value was inserted, false if it already exists
        bool insert(const T& value) 
        {
            // Check if value already exists
            if (search(value) != nullptr) 
            {
                return false;
            }
            
            Node* new_node = new Node(value);
            Node* y = nullptr;
            Node* x = root;
            
            while (x != nullptr) 
            {
                y = x;
                if (value < x->value) 
                {
                    x = x->left;
                } 
                else 
                {
                    x = x->right;
                }
            }
            
            new_node->parent = y;
            if (y == nullptr) 
            {
                root = new_node;
            } 
            else if (value < y->value) 
            {
                y->left = new_node;
            } 
            else 
            {
                y->right = new_node;
            }
            
            // Fix the tree
            if (new_node->parent == nullptr) 
            {
                new_node->color = BLACK;
                node_count++;
                return true;
            }
            
            if (new_node->parent->parent == nullptr) 
            {
                node_count++;
                return true;
            }
            
            fix_insert(new_node);
            node_count++;
            return true;
        }
    
        // Returns true if the value was erased, false if it wasn't found
        bool erase(const T& value) 
        {
            Node* z = search(value);
            if (z == nullptr) 
            {
                return false;
            }
            
            Node* y = z;
            Node* x = nullptr;
            Node* x_parent = nullptr;
            bool x_is_left_child = false;
            Color y_original_color = y->color;
            
            if (z->left == nullptr) 
            {
                x = z->right;
                x_parent = z->parent;
                x_is_left_child = (z->parent != nullptr && z == z->parent->left);
                transplant(z, z->right);
            } 
            else if (z->right == nullptr) 
            {
                x = z->left;
                x_parent = z->parent;
                x_is_left_child = (z->parent != nullptr && z == z->parent->left);
                transplant(z, z->left);
            } 
            else 
            {
                y = minimum(z->right);
                y_original_color = y->color;
                x = y->right;
                
                if (y->parent == z) 
                {
                    if (x != nullptr) {
                        x->parent = y;
                    }
                    x_parent = y;
                    x_is_left_child = false; // x is a right child of y
                } 
                else 
                {
                    x_parent = y->parent;
                    x_is_left_child = (y == y->parent->left);
                    transplant(y, y->right);
                    y->right = z->right;
                    if (y->right != nullptr) 
                    {
                        y->right->parent = y;
                    }
                }
                
                transplant(z, y);
                y->left = z->left;
                if (y->left != nullptr) 
                {
                    y->left->parent = y;
                }
                y->color = z->color;
            }
            
            if (y_original_color == BLACK) 
            {
                fix_erase(x, x_parent, x_is_left_child);
            }
            
            delete z;
            node_count--;
            return true;
        }
    
        // Verifies that the tree satisfies all red-black tree properties
        bool is_valid_tree() const 
        {
            if (root == nullptr) 
            {
                return true;
            }
            
            // Property 1: Every node is either red or black. (Enforced by enum)
            
            // Property 2: The root is black
            if (root->color != BLACK) 
            {
                return false;
            }
            
            // Property 3: Every leaf (NULL) is black. (Implicitly true)
            
            // Property 4: If a node is red, then both its children are black.
            if (!check_red_property(root)) 
            {
                return false;
            }
            
            // Property 5: For each node, all simple paths from the node to descendant leaves 
            // contain the same number of black nodes.
            int height = 0;
            if (!check_black_height(root, height)) 
            {
                return false;
            }
            
            // Additionally, check that it's a valid binary search tree
            return is_binary_search_tree(root, nullptr, nullptr);
        }

        // Additional utility functions
        size_t size() const 
        {
            return node_count;
        }
    
        bool contains(const T& value) const 
        {
            return search(value) != nullptr;
        }
    
        bool empty() const 
        {
            return root == nullptr;
        }
    
        void clear() 
        {
            clear_recursive(root);
            root = nullptr;
            node_count = 0;
        }

        std::string to_string() const 
        {
            std::stringstream ss;
            ss << "RedBlackTree (size=" << node_count << "):" << std::endl;
            
            if (root == nullptr) 
            {
                ss << "    [Empty tree]" << std::endl;
                return ss.str();
            }
            
            // Use recursive helper to build the tree representation
            to_string_recursive(root, ss);
            
            return ss.str();
        }

    private:
        enum Color { RED, BLACK };
    
        struct Node 
        {
            T value;
            Color color;
            Node* left;
            Node* right;
            Node* parent;
    
            Node(const T& val, Color c = RED, Node* l = nullptr, Node* r = nullptr, Node* p = nullptr)
                : value(val), color(c), left(l), right(r), parent(p) {}
        };

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

        /**
         * Handle for graph node of this RB-tree. Allows utility getters and setters
         * for any field of the node.
         */
        struct rbnode_handle_t : public binary_node_handle_t<T, TAllocator>
        {
            using Base = binary_node_handle_t<T, TAllocator>;
            using Base::node;
            using Base::m_allocator;
            using Base::m_index;

            rbnode_handle_t() : Base() {}
            rbnode_handle_t(size_t inIndex, TAllocator* inAllocator)
                : Base(inIndex, inAllocator) {}

            inline Color color() const noexcept 
            {
                return rbnode().color;
            }

            inline void set_color(Color newColor)
            {
                rbnode().color = newColor;
            }

        private:
            rbnode_t& rbnode() const 
            {
                return *static_cast<rbnode_t>(&(*m_allocator)[m_index]);
            }
        };
    
        Node* root;
        size_t node_count;
    
        // Helper methods
        void rotate_left(Node* x) 
        {
            Node* y = x->right;
            x->right = y->left;
            
            if (y->left != nullptr) 
            {
                y->left->parent = x;
            }
            
            y->parent = x->parent;
            
            if (x->parent == nullptr) 
            {
                root = y;
            } 
            else if (x == x->parent->left)
            {
                x->parent->left = y;
            } 
            else 
            {
                x->parent->right = y;
            }
            
            y->left = x;
            x->parent = y;
        }
    
        void rotate_right(Node* x) 
        {
            Node* y = x->left;
            x->left = y->right;
            
            if (y->right != nullptr) 
            {
                y->right->parent = x;
            }
            
            y->parent = x->parent;
            
            if (x->parent == nullptr) 
            {
                root = y;
            } 
            else if (x == x->parent->left) 
            {
                x->parent->left = y;
            } 
            else 
            {
                x->parent->right = y;
            }
            
            y->right = x;
            x->parent = y;
        }
    
        void fix_insert(Node* k) 
        {
            Node* u;
            
            while (k->parent != nullptr && k->parent->color == RED) 
            {
                if (k->parent == k->parent->parent->right) 
                {
                    u = k->parent->parent->left;
                    if (u != nullptr && u->color == RED) 
                    {
                        u->color = BLACK;
                        k->parent->color = BLACK;
                        k->parent->parent->color = RED;
                        k = k->parent->parent;
                    }
                    else 
                    {
                        if (k == k->parent->left) 
                        {
                            k = k->parent;
                            rotate_right(k);
                        }
                        k->parent->color = BLACK;
                        k->parent->parent->color = RED;
                        rotate_left(k->parent->parent);
                    }
                } 
                else 
                {
                    u = k->parent->parent->right;
                    if (u != nullptr && u->color == RED) 
                    {
                        u->color = BLACK;
                        k->parent->color = BLACK;
                        k->parent->parent->color = RED;
                        k = k->parent->parent;
                    } 
                    else 
                    {
                        if (k == k->parent->right) 
                        {
                            k = k->parent;
                            rotate_left(k);
                        }
                        k->parent->color = BLACK;
                        k->parent->parent->color = RED;
                        rotate_right(k->parent->parent);
                    }
                }
                if (k == root) 
                {
                    break;
                }
            }
            root->color = BLACK;
        }
    
        Node* search(const T& value) const 
        {
            Node* current = root;
            while (current != nullptr) 
            {
                if (value == current->value) 
                {
                    return current;
                } 
                else if (value < current->value) 
                {
                    current = current->left;
                } 
                else 
                {
                    current = current->right;
                }
            }
            return nullptr;
        }
    
        Node* minimum(Node* node) const 
        {
            if (node == nullptr) 
            {
                return nullptr;
            }
            
            while (node->left != nullptr) 
            {
                node = node->left;
            }
            
            return node;
        }
    
        void transplant(Node* u, Node* v) 
        {
            if (u->parent == nullptr) 
            {
                root = v;
            } 
            else if (u == u->parent->left) 
            {
                u->parent->left = v;
            } 
            else 
            {
                u->parent->right = v;
            }
            
            if (v != nullptr) 
            {
                v->parent = u->parent;
            }
        }
    
        void fix_erase(Node* x, Node* x_parent, bool x_is_left_child) 
        {
            Node* w;
            
            while ((x == nullptr || x->color == BLACK) && x != root) 
            {
                if (x == nullptr) 
                {
                    if (x_is_left_child) 
                    {
                        w = x_parent->right;
                        if (w->color == RED) 
                        {
                            w->color = BLACK;
                            x_parent->color = RED;
                            rotate_left(x_parent);
                            w = x_parent->right;
                        }
                        
                        if ((w->left == nullptr || w->left->color == BLACK) && 
                            (w->right == nullptr || w->right->color == BLACK)) 
                        {
                            w->color = RED;
                            x = x_parent;
                            if (x->parent != nullptr) 
                            {
                                x_is_left_child = (x == x->parent->left);
                                x_parent = x->parent;
                            }
                        } 
                        else 
                        {
                            if (w->right == nullptr || w->right->color == BLACK) 
                            {
                                if (w->left != nullptr) 
                                {
                                    w->left->color = BLACK;
                                }
                                w->color = RED;
                                rotate_right(w);
                                w = x_parent->right;
                            }
                            
                            w->color = x_parent->color;
                            x_parent->color = BLACK;
                            if (w->right != nullptr) 
                            {
                                w->right->color = BLACK;
                            }
                            rotate_left(x_parent);
                            x = root;
                        }
                    } 
                    else 
                    {   
                        // x is a right child
                        w = x_parent->left;
                        if (w->color == RED) 
                        {
                            w->color = BLACK;
                            x_parent->color = RED;
                            rotate_right(x_parent);
                            w = x_parent->left;
                        }
                        
                        if ((w->right == nullptr || w->right->color == BLACK) && 
                            (w->left == nullptr || w->left->color == BLACK)) 
                        {
                            w->color = RED;
                            x = x_parent;
                            if (x->parent != nullptr) 
                            {
                                x_is_left_child = (x == x->parent->left);
                                x_parent = x->parent;
                            }
                        } 
                        else 
                        {
                            if (w->left == nullptr || w->left->color == BLACK) 
                            {
                                if (w->right != nullptr) 
                                {
                                    w->right->color = BLACK;
                                }
                                w->color = RED;
                                rotate_left(w);
                                w = x_parent->left;
                            }
                            
                            w->color = x_parent->color;
                            x_parent->color = BLACK;
                            if (w->left != nullptr) 
                            {
                                w->left->color = BLACK;
                            }
                            rotate_right(x_parent);
                            x = root;
                        }
                    }
                } 
                else 
                { 
                    // x is not nullptr
                    if (x == x->parent->left) 
                    {
                        w = x->parent->right;
                        if (w->color == RED) 
                        {
                            w->color = BLACK;
                            x->parent->color = RED;
                            rotate_left(x->parent);
                            w = x->parent->right;
                        }
                        
                        if ((w->left == nullptr || w->left->color == BLACK) && 
                            (w->right == nullptr || w->right->color == BLACK)) 
                        {
                            w->color = RED;
                            x = x->parent;
                        } 
                        else 
                        {
                            if (w->right == nullptr || w->right->color == BLACK) 
                            {
                                if (w->left != nullptr) 
                                {
                                    w->left->color = BLACK;
                                }
                                w->color = RED;
                                rotate_right(w);
                                w = x->parent->right;
                            }
                            
                            w->color = x->parent->color;
                            x->parent->color = BLACK;
                            if (w->right != nullptr) 
                            {
                                w->right->color = BLACK;
                            }
                            rotate_left(x->parent);
                            x = root;
                        }
                    } 
                    else 
                    { // x is a right child
                        w = x->parent->left;
                        if (w->color == RED) 
                        {
                            w->color = BLACK;
                            x->parent->color = RED;
                            rotate_right(x->parent);
                            w = x->parent->left;
                        }
                        
                        if ((w->right == nullptr || w->right->color == BLACK) && 
                            (w->left == nullptr || w->left->color == BLACK)) 
                        {
                            w->color = RED;
                            x = x->parent;
                        } 
                        else 
                        {
                            if (w->left == nullptr || w->left->color == BLACK) 
                            {
                                if (w->right != nullptr) 
                                {
                                    w->right->color = BLACK;
                                }
                                w->color = RED;
                                rotate_left(w);
                                w = x->parent->left;
                            }
                            
                            w->color = x->parent->color;
                            x->parent->color = BLACK;
                            if (w->left != nullptr) 
                            {
                                w->left->color = BLACK;
                            }
                            rotate_right(x->parent);
                            x = root;
                        }
                    }

                    x_parent = x->parent;
                    if (x_parent != nullptr) 
                    {
                        x_is_left_child = (x == x_parent->left);
                    }
                }
            }
            
            if (x != nullptr) 
            {
                x->color = BLACK;
            }
        }
    
        void clear_recursive(Node* node) 
        {
            if (node == nullptr) 
            {
                return;
            }
            
            clear_recursive(node->left);
            clear_recursive(node->right);
            delete node;
        }
    
        // Validation methods
        bool is_binary_search_tree(Node* node, T* min_value, T* max_value) const 
        {
            if (node == nullptr) 
            {
                return true;
            }
            
            if ((min_value != nullptr && node->value <= *min_value) || 
                (max_value != nullptr && node->value >= *max_value)) 
                {
                return false;
            }
            
            return is_binary_search_tree(node->left, min_value, &node->value) && 
                   is_binary_search_tree(node->right, &node->value, max_value);
        }
    
        bool check_red_property(Node* node) const 
        {
            if (node == nullptr) {
                return true;
            }
            
            if (node->color == RED) {
                if ((node->left != nullptr && node->left->color == RED) || 
                    (node->right != nullptr && node->right->color == RED)) {
                    return false;
                }
            }
            
            return check_red_property(node->left) && check_red_property(node->right);
        }
    
        bool check_black_height(Node* node, int& height) const 
        {
            if (node == nullptr) {
                height = 1; // Null nodes are considered BLACK
                return true;
            }
            
            int left_height = 0, right_height = 0;
            if (!check_black_height(node->left, left_height) || 
                !check_black_height(node->right, right_height)) {
                return false;
            }
            
            if (left_height != right_height) {
                return false;
            }
            
            height = left_height + (node->color == BLACK ? 1 : 0);
            return true;
        }

        void to_string_recursive(Node* node, std::stringstream& ss, int depth = 0) const 
        {
            if (node == nullptr) 
            {
                ss << std::string(depth * 4, ' ') << "NULL" << std::endl;
                return;
            }
            
            ss << std::string(depth * 4, ' ') << node->value << " (" 
               << (node->color == RED ? "RED" : "BLACK") << ")" << std::endl;
            
            to_string_recursive(node->left, ss, depth + 1);
            to_string_recursive(node->right, ss, depth + 1);
        }
    };
}
