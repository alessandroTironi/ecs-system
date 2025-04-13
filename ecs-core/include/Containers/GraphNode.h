#pragma once 

#include <cassert>
#include <optional>

namespace ecs 
{
    template<typename T, size_t N>
    struct graph_node_t
    {
        T value;
        std::optional<size_t> parent;
        std::optional<size_t> children[N];

        graph_node_t()  
            : parent{std::nullopt}
        {
            for (size_t i = 0; i < N; ++i)
            {
                children[i] = std::nullopt;
            }
        }

        graph_node_t(T inValue, size_t inParent, size_t inChildren[N])
            : value{inValue}, parent{inParent}, children{inChildren}
        {}
    };
}