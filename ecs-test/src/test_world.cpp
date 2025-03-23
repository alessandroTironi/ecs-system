#include <gtest/gtest.h>
#include "World.h"
#include "Entity.h"
#include "ComponentData.h"
#include "ISystem.h"
#include "ArchetypeQuery.h"

using ::testing::Test; 

class TestECSWorld : public Test 
{
public:
    struct Position : public ecs::IComponent 
    {
    public:
        ecs::real_t x = 0.0f;
        ecs::real_t y = 0.0f;
    };

    struct Velocity : public ecs::IComponent 
    {
    public:
        ecs::real_t x = 0.0f;
        ecs::real_t y = 0.0f;
    };

    struct Rotation : public ecs::IComponent 
    {
    public:
        ecs::real_t angle = 0.0f;
    };

    class PhysicsSystem : public ecs::ISystem 
    {
    public:
        void Update(std::weak_ptr<ecs::World> world, ecs::real_t deltaTime) override
        {
            ecs::query<Position, Velocity>::MakeQuery(world).forEach(
                [deltaTime](ecs::EntityHandle entity, Position& position, Velocity& velocity)
                {
                    position.x += velocity.x * deltaTime;
                    position.y += velocity.y * deltaTime;
                });
        }
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

TEST_F(TestECSWorld, TestCreateEntityWithComponents)
{
    ecs::entity_id entity = m_world->CreateEntity<Position, Velocity>();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    EXPECT_NE(entityHandle.FindComponent<Position>(), nullptr);
    EXPECT_NE(entityHandle.FindComponent<Velocity>(), nullptr);
}

TEST_F(TestECSWorld, TestUpdateEntityArchetype)
{
    ecs::entity_id entity = m_world->CreateEntity<Position>();
    ecs::entity_id entity2 = m_world->CreateEntity<Position, Velocity>();
    ecs::EntityHandle entityHandle = m_world->GetEntity(entity);
    ecs::EntityHandle entityHandle2 = m_world->GetEntity(entity2);
    const ecs::archetype_id& originalArchetypeID = entityHandle.archetypeID();

    entityHandle.AddComponent<Velocity>();
    EXPECT_NE(entityHandle.archetypeID(), originalArchetypeID)
        << "Adding a component to an entity should update the entity handle's archetype ID.";
    EXPECT_EQ(entityHandle.archetypeID(), entityHandle2.archetypeID())
        << "When updating an entity handle's archetype id, it should be consistent within the archetype registry.";
}

TEST_F(TestECSWorld, TestAddSystem)
{
    EXPECT_EQ(m_world->GetSystemsCount(), 0);
    std::shared_ptr<PhysicsSystem> system = m_world->AddSystem<PhysicsSystem>();
    EXPECT_NE(system.get(), nullptr);
    EXPECT_EQ(m_world->GetSystemsCount(), 1);
}

TEST_F(TestECSWorld, TestGetSystem)
{
    ASSERT_THROW(m_world->GetSystem<PhysicsSystem>(), std::out_of_range);   
    m_world->AddSystem<PhysicsSystem>();
    EXPECT_NE(m_world->GetSystem<PhysicsSystem>(), nullptr);
}

TEST_F(TestECSWorld, TestFindSystem)
{
    EXPECT_EQ(m_world->FindSystem<PhysicsSystem>(), nullptr);
    m_world->AddSystem<PhysicsSystem>();
    EXPECT_NE(m_world->FindSystem<PhysicsSystem>(), nullptr);
}

TEST_F(TestECSWorld, TestRemoveSystem)
{
    std::shared_ptr<PhysicsSystem> system = m_world->AddSystem<PhysicsSystem>();
    EXPECT_EQ(m_world->GetSystemsCount(), 1);
    m_world->RemoveSystem<PhysicsSystem>();
    EXPECT_EQ(m_world->GetSystemsCount(), 0);
    EXPECT_EQ(m_world->FindSystem<PhysicsSystem>(), nullptr);
}

TEST_F(TestECSWorld, TestMakeQueries)
{
    m_world->CreateEntity<Position, Velocity>();
    m_world->CreateEntity<Position>();

    size_t count = 0;
    const auto countEntities = [&count](ecs::EntityHandle entity, Position&)
    {
        ++count;
    };

    ecs::query<Position>::MakeQuery(m_world).forEach(countEntities);
    EXPECT_EQ(count, 2) << "Queries from World should work as expected.";
}

TEST_F(TestECSWorld, TestUpdate)
{
    ecs::entity_id e1 = m_world->CreateEntity<Position, Velocity>();
    ecs::entity_id e2 = m_world->CreateEntity<Position, Velocity, Rotation>();

    ecs::EntityHandle e1Handle = m_world->GetEntity(e1);
    e1Handle.GetComponent<Position>().x = 0.0f;
    e1Handle.GetComponent<Position>().y = 0.0f;
    e1Handle.GetComponent<Velocity>().x = 1.0f;
    e1Handle.GetComponent<Velocity>().y = -4.0f; 

    ecs::EntityHandle e2Handle = m_world->GetEntity(e2);
    e2Handle.GetComponent<Position>().x = 0.0f;
    e2Handle.GetComponent<Position>().y = 0.0f;
    e2Handle.GetComponent<Velocity>().x = 2.0f;
    e2Handle.GetComponent<Velocity>().y = 0.0f; 

    m_world->AddSystem<PhysicsSystem>();

    m_world->Update(1.0f);

    EXPECT_EQ(e1Handle.GetComponent<Position>().x, 1.0f);
    EXPECT_EQ(e1Handle.GetComponent<Position>().y, -4.0f);
    EXPECT_EQ(e2Handle.GetComponent<Position>().x, 2.0f);
    EXPECT_EQ(e2Handle.GetComponent<Position>().y, 0.0f);

    m_world->Update(1.0f);

    EXPECT_EQ(e1Handle.GetComponent<Position>().x, 2.0f);
    EXPECT_EQ(e1Handle.GetComponent<Position>().y, -8.0f);
    EXPECT_EQ(e2Handle.GetComponent<Position>().x, 4.0f);
    EXPECT_EQ(e2Handle.GetComponent<Position>().y, 0.0f);
}
