#include "ComponentsDatabase.h"

ecs::IDGenerator<ecs::component_id> ecs::ComponentsDatabase::s_componentIDGenerator;
std::unordered_map<ecs::name, ecs::component_data> ecs::ComponentsDatabase::s_componentsClassMap;
std::vector<ecs::name> ecs::ComponentsDatabase::s_componentNames;

ecs::component_id ecs::ComponentsDatabase::AddComponentData(const ecs::name& componentName, 
	const size_t dataSize, const size_t initialCapacity)
{
	auto optionalComponentData = s_componentsClassMap.find(componentName);
	if (optionalComponentData == s_componentsClassMap.end())
	{
		const component_id newID = s_componentIDGenerator.GenerateNewUniqueID();
		s_componentsClassMap.emplace(componentName, component_data(dataSize, newID, initialCapacity));
		if (newID >= s_componentNames.size())
		{
			s_componentNames.resize(newID + 8);
		}
		s_componentNames[newID] = componentName;
		return newID;
	}
	else
	{
		return optionalComponentData->second.serial();
	}
}

bool ecs::ComponentsDatabase::TryGetComponentData(const ecs::name& componentName, component_data& outComponentData)
{
	auto optionalComponentData = s_componentsClassMap.find(componentName);
	if (optionalComponentData == s_componentsClassMap.end())
	{
		return false;
	}
	else
	{
		outComponentData = optionalComponentData->second;
		return true;
	}
}

bool ecs::ComponentsDatabase::TryGetComponentData(const component_id componentID, name& outComponentName, component_data& outComponentData)
{
	if (componentID >= s_componentNames.size())
	{
		return false;
	}

	outComponentName = s_componentNames[componentID];
	outComponentData = s_componentsClassMap[outComponentName];
	return true;
}