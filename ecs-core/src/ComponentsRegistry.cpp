#include "ComponentsRegistry.h"

ecs::component_id ecs::ComponentsRegistry::AddComponentData(const ecs::type_key& componentType, 
	const size_t dataSize, const size_t initialCapacity)
{
	auto optionalComponentData = m_componentsClassMap.find(componentType);
	if (optionalComponentData == m_componentsClassMap.end())
	{
		const component_id newID = m_componentIDGenerator.GenerateNewUniqueID();
		m_componentsClassMap.emplace(componentType, component_data(dataSize, newID, initialCapacity));
		if (newID >= m_componentTypes.size())
		{
			m_componentTypes.resize(newID + 8);
		}
		m_componentTypes[newID] = componentType;
		return newID;
	}
	else
	{
		return optionalComponentData->second.serial();
	}
}

bool ecs::ComponentsRegistry::TryGetComponentData(const ecs::type_key& componentType, component_data& outComponentData)
{
	auto optionalComponentData = m_componentsClassMap.find(componentType);
	if (optionalComponentData == m_componentsClassMap.end())
	{
		return false;
	}
	else
	{
		outComponentData = optionalComponentData->second;
		return true;
	}
}

bool ecs::ComponentsRegistry::TryGetComponentData(const component_id componentID, type_key& outComponentType, component_data& outComponentData)
{
	if (componentID >= m_componentTypes.size())
	{
		return false;
	}

	outComponentType = m_componentTypes[componentID];
	outComponentData = m_componentsClassMap[outComponentType];
	return true;
}