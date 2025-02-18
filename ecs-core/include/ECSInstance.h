#pragma once 

namespace ecs 
{
    class Instance 
    {
    public:
        Instance();
        ~Instance() = default;

        void Initialize();
        void Update(float deltaTime);
    };
}