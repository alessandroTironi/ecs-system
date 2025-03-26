#pragma once 

#include <memory>
#include "Types.h"
#include "Entity.h"
#include "ComponentsRegistry.h"

namespace ecs 
{
    class World;

    struct query_base 
    {
    public:
        query_base() = default;
        query_base(std::weak_ptr<World> world) : m_world(world) {}
    protected:
        std::weak_ptr<World> m_world;
    };
}