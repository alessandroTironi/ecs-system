#include "ComponentsRegistry.h"

ecs::component_id ecs::ComponentsRegistry::AddComponentData(const ecs::name& componentName, 
	const size_t dataSize, const size_t initialCapacity)
{
	auto optionalComponentData = m_componentsClassMap.find(componentName);
	if (optionalComponentData == m_componentsClassMap.end())
	{
		const component_id newID = m_componentIDGenerator.GenerateNewUniqueID();
		m_componentsClassMap.emplace(componentName, component_data(dataSize, newID, initialCapacity));
		if (newID >= m_componentNames.size())
		{
			m_componentNames.resize(newID + 8);
		}
		m_componentNames[newID] = componentName;
		return newID;
	}
	else
	{
		return optionalComponentData->second.serial();
	}
}

bool ecs::ComponentsRegistry::TryGetComponentData(const ecs::name& componentName, component_data& outComponentData)
{
	auto optionalComponentData = m_componentsClassMap.find(componentName);
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

bool ecs::ComponentsRegistry::TryGetComponentData(const component_id componentID, name& outComponentName, component_data& outComponentData)
{
	if (componentID >= m_componentNames.size())
	{
		return false;
	}

	outComponentName = m_componentNames[componentID];
	outComponentData = m_componentsClassMap[outComponentName];
	return true;
}