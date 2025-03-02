#include <gtest/gtest.h>
#include "ECSInstance.h"
#include "ISystem.h"
#include "ComponentArray.h"
#include <stdexcept>

class MockSystem : public ecs::ISystem
{
    void Update(ecs::real_t deltaTime) override {}
};

struct MockComponent : public ecs::IComponent 
{
public:
    MockComponent() {}
    MockComponent(float aFloat) : m_float(aFloat) {}

    float m_float = 0.0f;
};

struct IntComponent : public ecs::IComponent 
{
public:
    IntComponent() {}

    int m_int = 0;
};

TEST(TestECSInstance, TestConstructor)
{
    ecs::Instance* instance = nullptr;
    ASSERT_NO_THROW(instance = new ecs::Instance());
    delete instance;
}

TEST(TestECSInstance, TestInitialize)
{
    ecs::Instance* instance = new ecs::Instance();
    ASSERT_NO_THROW(instance->Initialize());

    delete instance;
}

TEST(TestECSInstance, TestAddSystems)
{
    std::shared_ptr<MockSystem> mockSystem = std::shared_ptr<MockSystem>(new MockSystem());
    ecs::Instance instance = ecs::Instance();
    ASSERT_EQ(instance.GetSystemsCount(), 0);

    ASSERT_NO_THROW(instance.AddSystem(mockSystem));
    ASSERT_NO_THROW(instance.AddSystem(mockSystem));
    ASSERT_EQ(instance.GetSystemsCount(), 1);
}

TEST(TestECSInstance, TestRemoveSystems)
{
    std::shared_ptr<MockSystem> mockSystem = std::shared_ptr<MockSystem>(new MockSystem());
    ecs::Instance instance = ecs::Instance();
    ASSERT_EQ(instance.GetSystemsCount(), 0);

    instance.AddSystem(mockSystem);
    ASSERT_EQ(instance.GetSystemsCount(), 1);

    instance.RemoveSystem(mockSystem);
    ASSERT_EQ(instance.GetSystemsCount(), 0);

    ASSERT_NO_THROW(instance.RemoveSystem(mockSystem));
}

TEST(TestECSInstance, TestUpdate)
{
    static constexpr float frameTime30 = 1.0f / 30.0f;
    ecs::Instance* instance = new ecs::Instance();
    ASSERT_NO_THROW(instance->Update(frameTime30));

    std::shared_ptr<MockSystem> mockSystem = std::shared_ptr<MockSystem>(new MockSystem());
    instance->AddSystem(mockSystem);

    ASSERT_NO_THROW(instance->Update(frameTime30));

    delete instance;
}

TEST(TestECSInstance, TestAddEntity)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1, e2;
    ASSERT_NO_THROW(instance.AddEntity(e1));
    ASSERT_EQ(instance.GetNumActiveEntities(), 1);

    ASSERT_NO_THROW(instance.AddEntity(e2));
    ASSERT_EQ(instance.GetNumActiveEntities(), 2);
    ASSERT_FALSE(e1 == e2);

    while (instance.GetNumActiveEntities() < instance.GetMaxNumEntities())
    {
        ecs::entity_id dumb;
        instance.AddEntity(dumb);
    }

    ecs::entity_id dumb2;
    ASSERT_THROW(instance.AddEntity(dumb2), std::exception);
}

TEST(TestECSInstance, TestGetEntity)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1, e2, e3;
    instance.AddEntity(e1);

    ASSERT_NO_THROW(instance.GetEntity(e1));

    instance.AddEntity(e2);
    instance.AddEntity(e3);
    instance.RemoveEntity(e2);

    ASSERT_NO_THROW(instance.GetEntity(e1));
    ASSERT_NO_THROW(instance.GetEntity(e3));
    ASSERT_THROW(instance.GetEntity(e2), std::out_of_range);
}

TEST(TestECSInstance, TestRemoveEntity)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1 = 0;
    ASSERT_NO_THROW(instance.RemoveEntity(e1));
    ASSERT_EQ(instance.GetNumActiveEntities(), 0);

    instance.AddEntity(e1);
    ASSERT_NO_THROW(instance.RemoveEntity(e1));
    ASSERT_EQ(instance.GetNumActiveEntities(), 0);
}

TEST(TestECSInstance, TestAddComponent)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1, e2;

    instance.AddEntity(e1);
    instance.AddEntity(e2);

    ASSERT_NO_THROW(IntComponent& c1 = instance.AddComponent<IntComponent>(e1));
    ASSERT_NO_THROW(IntComponent& c2 = instance.AddComponent<IntComponent>(e2));
    ASSERT_THROW(IntComponent& c3 = instance.AddComponent<IntComponent>(e1), std::invalid_argument);
}

TEST(TestECSInstance, TestGetComponent)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1, e2, e3, e4;

    instance.AddEntity(e1);
    instance.AddEntity(e2);
    instance.AddEntity(e3);
    instance.AddEntity(e4);

    instance.AddComponent<IntComponent>(e1);
    instance.AddComponent<IntComponent>(e3);

    ASSERT_NO_THROW(IntComponent& c1 = instance.GetComponent<IntComponent>(e1));
    ASSERT_NO_THROW(IntComponent& c3 = instance.GetComponent<IntComponent>(e3));
    ASSERT_THROW(IntComponent& c2 = instance.GetComponent<IntComponent>(e2), std::out_of_range);
}

TEST(TestECSInstance, TestRemoveComponent)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1, e2;
    instance.AddEntity(e1);
    instance.AddEntity(e2);

    instance.AddComponent<IntComponent>(e1);
    ASSERT_NO_THROW(instance.RemoveComponent<IntComponent>(e2));
    ASSERT_NO_THROW(instance.RemoveComponent<IntComponent>(e1));

    ASSERT_THROW(instance.GetComponent<IntComponent>(e1), std::out_of_range);
}

TEST(TestECSInstance, TestRemoveAllComponents)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e;
    instance.AddEntity(e);

    ASSERT_EQ(instance.GetNumComponents<IntComponent>(), 0);
    instance.AddComponent<IntComponent>(e);
    ASSERT_EQ(instance.GetNumComponents<IntComponent>(), 1);

    instance.RemoveEntity(e);
    ASSERT_EQ(instance.GetNumComponents<IntComponent>(), 0);
    //ASSERT_THROW(instance.GetComponent<IntComponent>(e), std::out_of_range);
}

TEST(TestECSInstance, TestDoesComponentExist)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e;
    instance.AddEntity(e);

    instance.AddComponent<IntComponent>(e);
    const ecs::type_hash_t intComponentType = GetTypeHash(IntComponent);
    const ecs::type_hash_t mockComponentType = GetTypeHash(MockComponent);
    ASSERT_TRUE(instance.DoesComponentExist<IntComponent>(e));
    ASSERT_FALSE(instance.DoesComponentExist<MockComponent>(e));

    instance.RemoveComponent<IntComponent>(e);
    ASSERT_FALSE(instance.DoesComponentExist<IntComponent>(e));
}