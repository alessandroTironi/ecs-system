#include <gtest/gtest.h>
#include <stdexcept>
#include <algorithm>
#include "Archetypes.h"
#include "ArchetypesRegistry.h"
#include "PackedComponentArray.h"
#include "ComponentData.h"

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
        m_componentsRegistry = std::make_shared<ecs::ComponentsRegistry>();
        m_archetypesRegistry = std::make_shared<ecs::ArchetypesRegistry>(m_componentsRegistry);
        m_emptyArchetype = ecs::archetype();
        m_archetype1 = ecs::archetype::make<FloatComponent>(m_componentsRegistry.get());
        m_archetype2 = ecs::archetype::make<FloatComponent, IntComponent>(m_componentsRegistry.get());
        m_archetype3 = ecs::archetype::make<FloatComponent, IntComponent, DoubleComponent>(m_componentsRegistry.get());
    }

    void TearDown() override
    {
        m_archetypesRegistry.reset();
        m_componentsRegistry.reset();
    }

    ecs::archetype m_emptyArchetype;
    ecs::archetype m_archetype1;
    ecs::archetype m_archetype2;
    ecs::archetype m_archetype3;
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
    std::set<ecs::component_id> components = {};
    ASSERT_THROW(ecs::archetype(std::move(components)), std::invalid_argument) <<   "It should not be possible to create an empty archetype: "
                                                                                    "An invalid_argument exception should be thrown";
    std::set<ecs::component_id> components2 = { 0 };
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
    std::vector<ecs::type_hash_t> componentIDs =
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
        const ecs::type_hash_t thisHash = *componentsIt;
        EXPECT_EQ(thisHash, componentIDs[index++]) 
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
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent).name(), floatComponentData));
    ASSERT_NO_THROW(packedArray1 = ecs::packed_component_array_t(floatComponentData));
    ASSERT_EQ(packedArray1.component_size(), sizeof(FloatComponent));
    ASSERT_EQ(packedArray1.size(), 0);
    ASSERT_EQ(packedArray1.component_serial(), m_componentsRegistry->GetComponentID<FloatComponent>());
    ASSERT_EQ(packedArray1.capacity(), 8);
}

TEST_F(TestArchetypes, TestAddComponentToPackedArray)
{
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent).name(), floatComponentData));
    ecs::packed_component_array_t packedArray(floatComponentData);
    void* newComponent = packedArray.add_component();
    EXPECT_EQ(packedArray.size(), 1);
    ASSERT_NE(newComponent, nullptr) << "Adding a component to a packed array should always return a valid pointer";
}

TEST_F(TestArchetypes, TestAddComponentRealloc)
{
    ecs::component_data floatComponentData;
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent).name(), floatComponentData));
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
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent).name(), floatComponentData));
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
    ASSERT_TRUE(m_componentsRegistry->TryGetComponentData(typeid(FloatComponent).name(), floatComponentData));
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
    EXPECT_EQ(ecs::CalculateArchetypeHash(m_archetypesRegistry->GetArchetype(0)), 
        ecs::CalculateArchetypeHash(floatArchetype));
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
    ASSERT_NO_THROW(m_archetypesRegistry->RemoveComponent<FloatComponent>(0))
        << "Removing a non-existing component should not throw any exception, but just do nothing";
    
    m_archetypesRegistry->AddComponent<FloatComponent>(0);
    m_archetypesRegistry->RemoveComponent<FloatComponent>(0);
    ASSERT_THROW(m_archetypesRegistry->GetComponent<FloatComponent>(0), std::out_of_range);
}
