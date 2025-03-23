#pragma once 

#include <memory>
#include "Types.h"

namespace ecs
{
    class World;

    class ISystem
    {
    public:
        virtual void Update(std::weak_ptr<World> world, real_t deltaTime) {}
    };
}