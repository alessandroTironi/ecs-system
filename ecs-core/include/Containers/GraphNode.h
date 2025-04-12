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
}