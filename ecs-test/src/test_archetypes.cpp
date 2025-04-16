#include <gtest/gtest.h>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include "Core/World.h"
#include "Core/Entity.h"
#include "Core/Archetypes.h"
#include "Core/ArchetypesRegistry.h"
#include "Core/PackedComponentArray.h"
#include "Core/ComponentData.h"

using ::testing::Test;

class TestArchetypes : public Test 
{
public:
    struct FloatComponent : public ecs::IComponent 
    {
    public:
        FloatComponent() {}
        FloatComponent(float value) : m_value(value) {}

        float m_value = 0.0f;
    };

    struct DoubleComponent : public ecs::IComponent 
    {
    public:
        DoubleComponent() {}
        DoubleComponent(double value) : m_value(value) {}

        double m_value = 0.0;
    };

    struct IntComponent : public ecs::IComponent 
    {
    public:
        IntComponent() {}
        IntComponent(int value) : m_value(value) {}

        int m_value = 0;
    };

protected:
    void SetUp() override
    {
        m_world = std::make_shared<ecs::World>();
        m_world->Initialize();
        m_componentsRegistry = m_world->GetComponentsRegistry();
        m_archetypesRegistry = m_world->GetArchetypesRegistry();
        //m_emptyArchetype = ecs::archetype();
        m_archetype1 = ecs::archetype::make<FloatComponent>(m_componentsRegistry.get());
        m_archetype2 = ecs::archetype::make<FloatComponent, IntComponent>(m_componentsRegistry.get());
        m_archetype3 = ecs::archetype::make<FloatComponent, IntComponent, DoubleComponent>(m_componentsRegistry.get());
    }

    void TearDown() override
    {
        m_archetypesRegistry.reset();
        m_componentsRegistry.reset();
        m_world.reset();
    }

    ecs::archetype m_emptyArchetype;
    ecs::archetype m_archetype1;
    ecs::archetype m_archetype2;
    ecs::archetype m_archetype3;
    std::shared_ptr<ecs::World> m_world;
    std::shared_ptr<ecs::ArchetypesRegistry> m_archetypesRegistry;
    std::shared_ptr<ecs::ComponentsRegistry> m_componentsRegistry;
};

TEST_F(TestArchetypes, TestNullArchetype)
{
    EXPECT_TRUE(m_emptyArchetype.is_null()) << "An empty archetype should always be null";
}

TEST_F(TestArchetypes, TestEmptyArchetype)
{
    ASSERT_THROW(ecs::archetype::make({}), std::invalid_argument) <<   "It should not be possible to create an empty archetype: "
                                                                            "An invalid_argument exception should be thrown";
    ASSERT_NO_THROW(ecs::archetype::make({0}));
}

TEST_F(TestArchetypes, TestVectorMoveConstructor)
{
    ecs::archetype::Signature components = {};
    ASSERT_THROW(ecs::archetype(std::move(components)), std::invalid_argument) <<   "It should not be possible to create an empty archetype: "
                                                                                    "An invalid_argument exception should be thrown";
    ecs::archetype::Signature components2 = { 0 };
    ASSERT_NO_THROW(ecs::archetype(components2));
}

TEST_F(TestArchetypes, TestTemplateMakeFunction)
{
    ASSERT_NO_THROW(ecs::archetype::make<FloatComponent>(m_componentsRegistry.get()));
    auto createTwoComponents = [&]() { ecs::archetype::make<FloatComponent, IntComponent>(m_componentsRegistry.get()); };
    ASSERT_NO_THROW(createTwoComponents());
}

TEST_F(TestArchetypes, TestNumComponents)
{
    EXPECT_EQ(m_emptyArchetype.get_num_components(), 0);
    EXPECT_EQ(m_archetype1.get_num_components(), 1);
    EXPECT_EQ(m_archetype2.get_num_components(), 2);
    EXPECT_EQ(m_archetype3.get_num_components(), 3);
}

TEST_F(TestArchetypes, TestIterator)
{
    ASSERT_NO_THROW(m_emptyArchetype.begin());
    ASSERT_NO_THROW(m_emptyArchetype.end());
    ASSERT_EQ(m_emptyArchetype.begin(), m_emptyArchetype.end()) <<  "For empty archetypes, begin() and end() should be equal";
    ASSERT_NO_THROW(m_archetype3.begin());
    ASSERT_NO_THROW(m_archetype3.end());
    ASSERT_NE(m_archetype3.begin(), m_archetype3.end()) <<  "For non-empty archetypes, begin() and end() should not be equal";
}

TEST_F(TestArchetypes, TestComponentsOrder)
{
    std::vector<ecs::component_id> componentIDs =
    {
        m_componentsRegistry->GetComponentID<FloatComponent>(),
        m_componentsRegistry->GetComponentID<DoubleComponent>(),
        m_componentsRegistry->GetComponentID<IntComponent>(),
    };

    std::sort(componentIDs.begin(), componentIDs.end());
    ASSERT_TRUE(componentIDs[0] < componentIDs[1] && componentIDs[1] < componentIDs[2]);

    size_t index = 0;
    for (auto componentsIt = m_archetype3.begin(); componentsIt != m_archetype3.end(); ++componentsIt)
    {
        const ecs::component_id thisID = *componentsIt;
        EXPECT_EQ(thisID, componentIDs[index++]) 
            <<  "Components of an archetype should always be sorted by their serial ID, "
                "to ensure consistency of hashes.";
    }   
}

TEST_F(TestArchetypes, TestHashing)
{
    m_componentsRegistry->RegisterComponent<FloatComponent>();
    m_componentsRegistry->RegisterComponent<IntComponent>(); 
    m_componentsRegistry->RegisterComponent<DoubleComponent>();

    const auto hash1 = std::hash<ecs::archetype>{}(m_archetype1);
    const auto hash2 = std::hash<ecs::archetype>{}(m_archetype2);
    const auto hash3 = std::hash<ecs::archetype>{}(m_archetype3);

    EXPECT_NE(hash1, hash2) << "Different archetypes should have different hashes";
    EXPECT_NE(hash2, hash3) << "Different archetypes should have different hashes";
    EXPECT_NE(hash1, hash3) << "Different archetypes should have different hashes";

    EXPECT_EQ(hash1, std::hash<ecs::archetype>{}(ecs::archetype::make<FloatComponent>(m_componentsRegistry.get())))
        << "Archetypes with the same components should have the same hash";
    EXPECT_EQ(hash2, std::hash<ecs::archetype>{}(ecs::archetype::make<IntComponent, FloatComponent>(m_componentsRegistry.get())))
        << "Archetypes with the same components declared in different order should have the same hash";
}

TEST_F(TestArchetypes, TestPackedComponentArrayCreation)
{
    ecs::packed_component_array_t packedArray1;
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent), floatComponentData));
    ASSERT_NO_THROW(packedArray1 = ecs::packed_component_array_t(floatComponentData));
    ASSERT_EQ(packedArray1.component_size(), sizeof(FloatComponent));
    ASSERT_EQ(packedArray1.size(), 0);
    ASSERT_EQ(packedArray1.component_serial(), m_componentsRegistry->GetComponentID<FloatComponent>());
    ASSERT_EQ(packedArray1.capacity(), 8);
}

TEST_F(TestArchetypes, TestAddComponentToPackedArray)
{
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent), floatComponentData));
    ecs::packed_component_array_t packedArray(floatComponentData);
    void* newComponent = packedArray.add_component();
    EXPECT_EQ(packedArray.size(), 1);
    ASSERT_NE(newComponent, nullptr) << "Adding a component to a packed array should always return a valid pointer";
}

TEST_F(TestArchetypes, TestAddComponentRealloc)
{
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent), floatComponentData));
    ecs::packed_component_array_t packedArray(floatComponentData);
    void* newComponent1 = packedArray.add_component();
    void* newComponent2 = packedArray.add_component();
    void* newComponent3 = packedArray.add_component();
    void* newComponent4 = packedArray.add_component();
    void* newComponent5 = packedArray.add_component();
    void* newComponent6 = packedArray.add_component();
    void* newComponent7 = packedArray.add_component();
    void* newComponent8 = packedArray.add_component();
    void* newComponent9 = packedArray.add_component();

    ASSERT_NE(newComponent3, nullptr) 
        << "Adding a component to a packed array that reached the maximum capacity should reallocate memory and return a valid pointer";
    EXPECT_TRUE(packedArray.size() <= packedArray.capacity()) 
        << "The size of the packed array should always be less than or equal to its capacity";
    ASSERT_EQ(packedArray.capacity(), 16) 
        << "The capacity of the packed array should be doubled when it reaches its limit";
}

TEST_F(TestArchetypes, TestGetComponentFromPackedArray)
{
    m_componentsRegistry->RegisterComponent<FloatComponent>(2);
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent), floatComponentData));
    ecs::packed_component_array_t packedArray(floatComponentData);
    void* newComponent1 = packedArray.add_component();
    void* newComponent2 = packedArray.add_component();

    EXPECT_EQ(newComponent1, packedArray.get_component(0));
    EXPECT_EQ(newComponent2, packedArray.get_component(1));
    ASSERT_THROW(packedArray.get_component(2), std::out_of_range);
}

TEST_F(TestArchetypes, TestDeleteComponentFromPackedArray)
{
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent), floatComponentData));
    ecs::packed_component_array_t packedArray(floatComponentData);
    void* c1 = packedArray.add_component();
    void* c2 = packedArray.add_component();

    ASSERT_THROW(packedArray.delete_at(2), std::out_of_range)
        << "Attempting to delete a component at an invalid index should throw an out_of_range exception";

    packedArray.delete_at(1);
    EXPECT_EQ(packedArray.size(), 1);
    EXPECT_EQ(packedArray.capacity(), 8);
}

TEST_F(TestArchetypes, TestTemplatePackedComponentArray)
{
    ecs::packed_component_array<FloatComponent> packedArray(m_componentsRegistry.get());
    FloatComponent& component = packedArray.add_component();
    component.m_value = 3.14f;
    ASSERT_NEAR(packedArray.get_component(0).m_value, 3.14f, 0.0001f);
}

TEST_F(TestArchetypes, TestEmplaceComponentInPackedArray)
{
    ecs::packed_component_array<FloatComponent> packedArray(m_componentsRegistry.get());
    FloatComponent& component = packedArray.emplace_component(3.14f);
    ASSERT_NEAR(packedArray.get_component(0).m_value, 3.14f, 0.0001f);
}

TEST_F(TestArchetypes, TestAddEntity)
{
    ASSERT_NO_THROW(m_archetypesRegistry->AddEntity<FloatComponent>(0));
    EXPECT_EQ(m_archetypesRegistry->GetNumArchetypes(), 1);

    ASSERT_NO_THROW(m_archetypesRegistry->AddEntity<>(1));
}

TEST_F(TestArchetypes, TestGetComponentFromArchetypesDatabase)
{
    m_archetypesRegistry->AddEntity<FloatComponent>(0);
    FloatComponent& component = m_archetypesRegistry->GetComponent<FloatComponent>(0);
    component.m_value = 3.0f;
    EXPECT_NEAR(m_archetypesRegistry->GetComponent<FloatComponent>(0).m_value, 3.0f, 0.0001f);

    ASSERT_THROW(FloatComponent& unexistingComponent = m_archetypesRegistry->GetComponent<FloatComponent>(1), std::out_of_range)
        <<  "Getting an lvalue reference to a non-existing component should throw an exception.";

    m_archetypesRegistry->AddEntity<IntComponent>(1);
    ASSERT_THROW(m_archetypesRegistry->GetComponent<FloatComponent>(1), std::out_of_range)
        <<  "Getting an lvalue reference to a non-existing component should throw an exception.";
}

TEST_F(TestArchetypes, TestRemoveEntityFromArchetypeDatabase)
{
    m_archetypesRegistry->AddEntity<FloatComponent>(0);
    ASSERT_NO_THROW(m_archetypesRegistry->RemoveEntity(0));

    ASSERT_THROW(m_archetypesRegistry->GetComponent<FloatComponent>(0), std::out_of_range);

    ASSERT_NO_THROW(m_archetypesRegistry->RemoveEntity(1));
}

TEST_F(TestArchetypes, TestGetArchetype)
{
    m_archetypesRegistry->AddEntity<FloatComponent>(0);
    const ecs::archetype& floatArchetype = ecs::archetype::make<FloatComponent>(m_componentsRegistry.get());
}

TEST_F(TestArchetypes, TestAddComponentToEntityInArchetypesDatabase)
{
    m_archetypesRegistry->AddEntity<FloatComponent>(0);
    ASSERT_NO_THROW(m_archetypesRegistry->AddComponent<IntComponent>(0));

    ASSERT_NO_THROW(m_archetypesRegistry->GetComponent<FloatComponent>(0));
    ASSERT_NO_THROW(m_archetypesRegistry->GetComponent<IntComponent>(0));

    ASSERT_TRUE(m_archetypesRegistry->GetArchetype(0).has_component(m_componentsRegistry->GetComponentID<FloatComponent>()));
    ASSERT_TRUE(m_archetypesRegistry->GetArchetype(0).has_component(m_componentsRegistry->GetComponentID<IntComponent>()));
}

TEST_F(TestArchetypes, TestRemoveComponentFromEntityInArchetypeDatabase)
{
    m_archetypesRegistry->AddEntity<IntComponent>(0);
    m_archetypesRegistry->AddEntity<IntComponent>(1);
    m_archetypesRegistry->AddEntity<IntComponent>(2);
    ASSERT_NO_THROW(m_archetypesRegistry->RemoveComponent<FloatComponent>(0))
        << "Removing a non-existing component should not throw any exception, but just do nothing";
    const ecs::archetype_id intArchetypeID = m_archetypesRegistry->GetArchetypeID(0);
    
    m_archetypesRegistry->AddComponent<FloatComponent>(2);
    const ecs::archetype_id intFloatArchetypeID = m_archetypesRegistry->GetArchetypeID(2);

    EXPECT_NE(intArchetypeID, intFloatArchetypeID);
    m_archetypesRegistry->RemoveComponent<FloatComponent>(2);
    ASSERT_THROW(m_archetypesRegistry->GetComponent<FloatComponent>(2), std::out_of_range);
    ASSERT_NO_THROW(m_archetypesRegistry->GetComponent<IntComponent>(2));
    EXPECT_EQ(intArchetypeID, m_archetypesRegistry->GetArchetypeID(2));
}

TEST_F(TestArchetypes, TestComplexUpdateOfArchetypes)
{
    m_archetypesRegistry->AddEntity<FloatComponent>(0);
    m_archetypesRegistry->AddEntity<FloatComponent>(1);
    m_archetypesRegistry->AddEntity<FloatComponent, IntComponent>(2);
    m_archetypesRegistry->AddEntity<IntComponent>(3);
    m_archetypesRegistry->AddEntity<FloatComponent, IntComponent, DoubleComponent>(4);

    const ecs::archetype_id floatArchetypeID = m_archetypesRegistry->GetArchetypeID(0);
    const ecs::archetype_id intArchetypeID = m_archetypesRegistry->GetArchetypeID(3);
    const ecs::archetype_id floatIntArchetypeID = m_archetypesRegistry->GetArchetypeID(2);
    const ecs::archetype_id floatIntDoubleArchetypeID = m_archetypesRegistry->GetArchetypeID(4);

    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(floatArchetypeID), 2);
    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(intArchetypeID), 1);
    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(floatIntArchetypeID), 1);
    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(floatIntDoubleArchetypeID), 1);

    m_archetypesRegistry->AddComponent<IntComponent>(0);
    m_archetypesRegistry->AddComponent<IntComponent>(1); 

    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(floatArchetypeID), 0);
    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(intArchetypeID), 1);
    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(floatIntArchetypeID), 3);
    EXPECT_EQ(m_archetypesRegistry->GetNumEntitiesForArchetype(floatIntDoubleArchetypeID), 1);

    EXPECT_EQ(m_archetypesRegistry->GetArchetypeID(0), floatIntArchetypeID);
    EXPECT_EQ(m_archetypesRegistry->GetArchetypeID(1), floatIntArchetypeID);

    size_t numFloats = 0;
    std::function<void(ecs::EntityHandle, FloatComponent&)> countFloats = 
        [&numFloats](ecs::EntityHandle entity, FloatComponent& floatComponent) { ++numFloats; };
    m_archetypesRegistry->ForEachEntity<FloatComponent>(countFloats);
    EXPECT_EQ(numFloats, 4);

    size_t numInts = 0;
    std::function<void(ecs::EntityHandle, IntComponent&)> countInts = 
        [&numInts](ecs::EntityHandle entity, IntComponent& intComponent) { ++numInts; };
    m_archetypesRegistry->ForEachEntity<IntComponent>(countInts);
    EXPECT_EQ(numInts, 5);
}
