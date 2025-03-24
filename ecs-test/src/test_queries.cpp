#include <gtest/gtest.h>
#include <memory>
#include <set>
#include "Types.h"
#include "Archetypes.h"
#include "World.h"
#include "ArchetypeQuery.h"
#include "Entity.h"

using ::testing::Test;

class TestArchetypeQueries : public Test 
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

    struct Scale : public ecs::IComponent 
    {
    public:
        float scale = 1.0f;
    };

protected:
    void SetUp() override
    {
        m_world = std::make_shared<ecs::World>();
        m_world->Initialize();
        m_entity1Pos = m_world->GetEntity(m_world->CreateEntity<Position>());
        m_entity2Pos = m_world->GetEntity(m_world->CreateEntity<Position>());
        m_entity1PosVel = m_world->GetEntity(m_world->CreateEntity<Position, Velocity>());
        m_entity1Vel = m_world->GetEntity(m_world->CreateEntity<Velocity>());
        m_entity1PosVelRot = m_world->GetEntity(m_world->CreateEntity<Position, Velocity, Rotation>());
    }

    void TearDown() override
    {
        m_world.reset();
    }

    template<typename... Components>
    int CountEntities()
    {
        ecs::query<Components...> query(m_world);
        int count = 0;
        const auto updateCounter = [&count](ecs::EntityHandle entity, Components&... components)
        {
            ++count;
        };
        query.forEach(updateCounter);
        return count;
    }

    ecs::EntityHandle m_entity1Pos;
    ecs::EntityHandle m_entity2Pos;
    ecs::EntityHandle m_entity1PosVel;
    ecs::EntityHandle m_entity1Vel;
    ecs::EntityHandle m_entity1PosVelRot;
    std::shared_ptr<ecs::World> m_world;
};

TEST_F(TestArchetypeQueries, TestCreateQuery)
{
    ASSERT_NO_THROW(ecs::query<Position>());
    ASSERT_NO_THROW(ecs::query<Position>(m_world));
}

TEST_F(TestArchetypeQueries, TestQueryWithNoWorld)
{
    ecs::query<Position> query;
    ASSERT_THROW(query.forEach([](ecs::EntityHandle, Position&) {}), std::runtime_error);
}

TEST_F(TestArchetypeQueries, TestNormalQuery)
{
    int count = CountEntities<Position>();
    EXPECT_EQ(count, 4);
}

TEST_F(TestArchetypeQueries, TestQueryWithMultipleComponents)
{
    int count = CountEntities<Position, Velocity>();
    EXPECT_EQ(count, 2);
}

TEST_F(TestArchetypeQueries, TestEmptyQuery)
{
    EXPECT_EQ(CountEntities<Scale>(), 0);
}

TEST_F(TestArchetypeQueries, TestQueryAllEntities)
{
    EXPECT_EQ(CountEntities(), 5);
}

TEST_F(TestArchetypeQueries, TestQueryValidity)
{
    std::set<ecs::entity_id> positionEntities =
    {
        m_entity1Pos.id(), m_entity2Pos.id(), m_entity1PosVel.id(), m_entity1PosVelRot.id()
    };

    ecs::query<Position>(m_world).forEach([&positionEntities](ecs::EntityHandle entity, Position& position)
    {
        EXPECT_TRUE(positionEntities.find(entity.id()) != positionEntities.end());
        positionEntities.erase(entity.id());
    });
    EXPECT_TRUE(positionEntities.empty());
}

TEST_F(TestArchetypeQueries, TestQueryThatModifiesComponents)
{
    ecs::query<Position>(m_world).forEach([](ecs::EntityHandle entity, Position& position)
    {
        position.x = 3.14f;
        position.y = 42.0f;
    });

    EXPECT_EQ(m_entity1Pos.GetComponent<Position>().x, 3.14f);
    EXPECT_EQ(m_entity1Pos.GetComponent<Position>().y, 42.0f);
    EXPECT_EQ(m_entity2Pos.GetComponent<Position>().x, 3.14f);
    EXPECT_EQ(m_entity2Pos.GetComponent<Position>().y, 42.0f);
    EXPECT_EQ(m_entity1PosVel.GetComponent<Position>().x, 3.14f);
    EXPECT_EQ(m_entity1PosVel.GetComponent<Position>().y, 42.0f);
    EXPECT_EQ(m_entity1PosVelRot.GetComponent<Position>().x, 3.14f);
    EXPECT_EQ(m_entity1PosVelRot.GetComponent<Position>().y, 42.0f);
}

TEST_F(TestArchetypeQueries, TestQueryThatModifiesEntities)
{
    int numPositionsBefore = CountEntities<Position>();
    int numVelocitiesBefore = CountEntities<Velocity>();
    ecs::query<Position>(m_world).forEach([](ecs::EntityHandle entity, Position& position)
    {
        position.x = 3.14f;
        
        if (entity.FindComponent<Velocity>() == nullptr)
        {
            entity.AddComponent<Velocity>();
        }
    });

    const int numPositions = CountEntities<Position>();
    const int numVelocities = CountEntities<Velocity>();
    EXPECT_EQ(numPositionsBefore, numPositions);

    ecs::query<Position>(m_world).forEach([](ecs::EntityHandle entity, Position& position)
    {
        EXPECT_NE(entity.FindComponent<Velocity>(), nullptr);
        EXPECT_NEAR(position.x, 3.14f, 0.0001f);
    });
}