#pragma once 

#include "Types.h"

namespace ecs
{
    class ISystem
    {
    public:
        virtual void Update(real_t deltaTime) {}
    };
}