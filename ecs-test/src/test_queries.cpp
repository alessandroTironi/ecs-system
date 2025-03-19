#include <gtest/gtest.h>
#include <memory>
#include "Types.h"
#include "Archetypes.h"
#include "ArchetypesRegistry.h"
#include "ComponentsRegistry.h"
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
        m_componentsRegistry = std::make_shared<ecs::ComponentsRegistry>();
        m_archetypesRegistry = ecs::ArchetypesRegistry(m_componentsRegistry);
        m_archetypesRegistry.AddEntity<Position>(0);
        m_archetypesRegistry.AddEntity<Position>(1);
        m_archetypesRegistry.AddEntity<Position, Velocity>(2);
        m_archetypesRegistry.AddEntity<Velocity>(3);
        m_archetypesRegistry.AddEntity<Position, Velocity, Rotation>(4);
    }

    void TearDown() override
    {
        m_archetypesRegistry.Reset();
        m_componentsRegistry.reset();
    }

    ecs::entity_id m_entity1Pos;
    ecs::entity_id m_entity2Pos;
    ecs::entity_id m_entity1PosVel;
    ecs::entity_id m_entity1Vel;
    ecs::entity_id m_entity1PosVelRot;
    ecs::ArchetypesRegistry m_archetypesRegistry; 
    std::shared_ptr<ecs::ComponentsRegistry> m_componentsRegistry;
};

TEST_F(TestArchetypeQueries, TestQueryOneComponent)
{
    ecs::query<Position> positionQuery;
}