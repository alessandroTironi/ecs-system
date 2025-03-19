#include <gtest/gtest.h>
#include "World.h"

using ::testing::Test; 

class TestECSWorld : public Test 
{
public:
    struct Position : public ecs::IComponent 
    {
    public:
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Velocity : public ecs::IComponent 
    {
    public:
        float x = 0.0f;
        float y = 0.0f;
    };

    struct Rotation : public ecs::IComponent 
    {
    public:
        float angle = 0.0f;
    };

protected:
    void SetUp() override
    {
        m_world = std::make_unique<ecs::World>();
	}

    void TearDown() override
    {
        m_world.reset();
    }

    std::unique_ptr<ecs::World> m_world;
};

TEST_F(TestECSWorld, TestCreateWorld)
{
	EXPECT_NE(m_world.get(), nullptr);
}


