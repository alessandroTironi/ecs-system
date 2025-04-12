#pragma once 

#include <cassert>

namespace ecs 
{
    template<typename T, size_t N>
    struct graph_node_t
    {
        T value;
        size_t parent;
        size_t children[N];

        graph_node_t()  = default;
        graph_node_t(T inValue, size_t inParent, size_t inChildren[N])
            : value{inValue}, parent{inParent}, children{inChildren}
        {}
    };

    template<typename T, size_t N, typename Allocator>
    struct node_handle_t 
    {
    public:
        using Handle = node_handle_t<T, N, Allocator>;

        inline size_t index() const noexcept { return m_index; }
        inline Allocator* allocator() const noexcept { return m_allocator; }

        inline graph_node_t<T, N> node() const 
        {
            assert(m_allocator != nullptr);
            // @todo assert index is valid
            return (*m_allocator)[m_index];
        }

        inline Handle parent() const 
        {
            return Handle(node().parent, m_allocator);
        }

        inline Handle child(size_t n) const 
        {
            return Handle(node().children[n], m_allocator);
        }

        inline void set_value(T value)
        {
            node().value = value;
        }

        inline void set_parent(Handle newParent) 
        {
            node().parent = newParent.index();
        }

        inline void set_child(size_t n, Handle newChild) 
        {
            node().children[n] = newChild.index();
        }

        bool operator==(const Handle& other) const 
        {
            return m_index == other.index() && m_allocator == other.m_allocator;
        }

        bool operator!=(const Handle& other) const 
        {
            return m_index != other.index() || m_allocator != other.m_allocator;
        }
        
    protected:
        size_t m_index;
        Allocator* m_allocator;
    };

    template<typename T, typename TAllocator>
    struct binary_node_handle_t : public node_handle_t<T, 2, TAllocator>
    {
        using Base = node_handle_t<T, 2, TAllocator>;
        using Base::child;
        using Base::m_allocator;

        binary_node_handle_t() : Base() {}
        binary_node_handle_t(size_t index, TAllocator* allocator)
            : Base(index, allocator)
        {}

        inline binary_node_handle_t<T, TAllocator> left() const 
        {
            return binary_node_handle_t<T, TAllocator>(child(0).index(), m_allocator);
        }

        inline binary_node_handle_t<T, TAllocator> right() const 
        {
            return binary_node_handle_t<T, TAllocator>(child(1).index(), m_allocator);
        }
    };
}