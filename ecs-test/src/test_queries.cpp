#include <gtest/gtest.h>
#include "Types.h"
#include "Archetypes.h"
#include "ArchetypesDatabase.h"
#include "ComponentsDatabase.h"
#include "ArchetypeQuery.h"

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

protected:
    void SetUp() override
    {
        m_archetypesDatabase.AddEntity<Position>(0);
        m_archetypesDatabase.AddEntity<Position>(1);
        m_archetypesDatabase.AddEntity<Position, Velocity>(2);
        m_archetypesDatabase.AddEntity<Velocity>(3);
        m_archetypesDatabase.AddEntity<Position, Velocity, Rotation>(4);
    }

    void TearDown() override
    {
        m_archetypesDatabase.Reset();
    }

    ecs::entity_id m_entity1Pos;
    ecs::entity_id m_entity2Pos;
    ecs::entity_id m_entity1PosVel;
    ecs::entity_id m_entity1Vel;
    ecs::entity_id m_entity1PosVelRot;
    ecs::ArchetypesDatabase m_archetypesDatabase; 
};

TEST_F(TestArchetypeQueries, TestQueryOneComponent)
{
    ecs::query<Position> positionQuery;
}