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

TEST(TestComponentArray, TestInitialization)
{
    ecs::component_array<MockComponent> componentArray;

    ASSERT_EQ(componentArray.GetNumComponents(), 0);
}

TEST(TestComponentArray, TestAddComponent)
{
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e1, e2;
    instance.AddEntity(e1);
    instance.AddEntity(e2);

    ecs::component_array<MockComponent> componentArray;
    MockComponent& component1 = componentArray.add_component(e1);
    ASSERT_EQ(componentArray.GetNumComponents(), 1);

    MockComponent& component2 = componentArray.add_component(e2);
    ASSERT_EQ(componentArray.GetNumComponents(), 2);
}

TEST(TestComponentArray, TestMaxCapacity)
{
    ecs::component_array<MockComponent> componentArray;
    for (size_t e = 0; e < componentArray.capacity(); ++e)
    {
        MockComponent& dumbComponent = componentArray.add_component(e);
    }

    ASSERT_THROW(componentArray.add_component(componentArray.capacity() + 1), std::exception);
}

TEST(TestComponentArray, TestMultipleComponents)
{
    ecs::component_array<MockComponent> componentArray;
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e;

    instance.AddEntity(e);
    MockComponent& c1 = componentArray.add_component(e);
    ASSERT_THROW(MockComponent& c2 = componentArray.add_component(e), std::invalid_argument);
}

TEST(TestComponentArray, TestRemoveUnexistingComponent)
{
    ecs::component_array<MockComponent> componentArray;
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e;

    instance.AddEntity(e);
    ASSERT_FALSE(componentArray.remove_component(e));
}

TEST(TestComponentArray, TestRemoveComponent)
{
    ecs::component_array<MockComponent> componentArray;
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e;

    instance.AddEntity(e);
    MockComponent& c = componentArray.add_component(e);
    ASSERT_TRUE(componentArray.remove_component(e));
    ASSERT_EQ(componentArray.GetNumComponents(), 0);
}

TEST(TestComponentArray, TestAddRemoveAddRemoveComponent)
{
    ecs::component_array<MockComponent> componentArray;
    ecs::Instance instance = ecs::Instance();
    ecs::entity_id e;

    instance.AddEntity(e);
    MockComponent& c = componentArray.add_component(e);
    componentArray.remove_component(e);
    ASSERT_NO_THROW(componentArray.add_component(e));
    ASSERT_EQ(componentArray.GetNumComponents(), 1);
    ASSERT_NO_THROW(componentArray.remove_component(e));
    ASSERT_EQ(componentArray.GetNumComponents(), 0);
}