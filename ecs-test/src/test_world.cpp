#include <gtest/gtest.h>
#include "World.h"
#include "Entity.h"
#include "ComponentData.h"

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
        m_world = std::make_shared<ecs::World>();
	}

    void TearDown() override
    {
        m_world.reset();
    }

    std::shared_ptr<ecs::World> m_world;
};

TEST_F(TestECSWorld, TestCreateWorld)
{
	EXPECT_NE(m_world.get(), nullptr);
}

TEST_F(TestECSWorld, TestCreateEntity)
{
	ecs::entity_id entity = m_world->CreateEntity();
	EXPECT_NE(entity, ecs::INVALID_ENTITY_ID);
}

TEST_F(TestECSWorld, TestGetEntity)
{
	ecs::entity_id entity = m_world->CreateEntity();
	ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
	EXPECT_EQ(entityHandle.id(), entity);
    EXPECT_EQ(entityHandle.world().lock().get(), m_world.get());
}

TEST_F(TestECSWorld, TestGetUnexistingComponent)
{
    ecs::entity_id entity = m_world->CreateEntity();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    ASSERT_THROW(entityHandle.GetComponent<Position>(), std::out_of_range);
}

TEST_F(TestECSWorld, TestAddComponent)
{
    ecs::entity_id entity = m_world->CreateEntity();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    ASSERT_NO_THROW(entityHandle.AddComponent<Position>());
}

TEST_F(TestECSWorld, TestGetComponent)
{
    ecs::entity_id entity = m_world->CreateEntity();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    entityHandle.AddComponent<Position>();
    ASSERT_NO_THROW(entityHandle.GetComponent<Position>());

    Position& position = entityHandle.GetComponent<Position>();
    position.x = 10.0f;
    EXPECT_NEAR(entityHandle.GetComponent<Position>().x, 10.0f, 0.0001f);
}

TEST_F(TestECSWorld, TestFindComponent)
{
    ecs::entity_id entity = m_world->CreateEntity();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    ASSERT_NO_THROW(entityHandle.FindComponent<Position>())
        << "Calling FindComponent should be safe and never throw exceptions even if the component is not found.";
    EXPECT_EQ(entityHandle.FindComponent<Position>(), nullptr);

    entityHandle.AddComponent<Position>();
    Position* pos = entityHandle.FindComponent<Position>();
    EXPECT_NE(pos, nullptr);
    pos->x = 3.14f;

    EXPECT_NEAR(entityHandle.FindComponent<Position>()->x, 3.14f, 0.00001f);
}

TEST_F(TestECSWorld, TestRemoveComponent)
{
    ecs::entity_id entity = m_world->CreateEntity();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    entityHandle.AddComponent<Position>();
    ASSERT_NO_THROW(entityHandle.RemoveComponent<Position>());
}

TEST_F(TestECSWorld, TestRemoveUnexistingComponent)
{
    ecs::entity_id entity = m_world->CreateEntity();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    ASSERT_NO_THROW(entityHandle.RemoveComponent<Position>());
}

