#include "ComponentsDatabase.h"

ecs::IDGenerator<ecs::component_id> ecs::ComponentsDatabase::s_componentIDGenerator;
std::unordered_map<size_t, ecs::component_data> ecs::ComponentsDatabase::s_componentsClassMap;

ecs::component_id ecs::ComponentsDatabase::AddComponentData(const type_hash_t componentHash, 
	const size_t dataSize, const size_t initialCapacity)
{
	auto optionalComponentData = s_componentsClassMap.find(componentHash);
	if (optionalComponentData == s_componentsClassMap.end())
	{
		const component_id newID = s_componentIDGenerator.GenerateNewUniqueID();
		s_componentsClassMap.emplace(componentHash, component_data(componentHash, dataSize, newID, initialCapacity));
		return newID;
	}
	else
	{
		return optionalComponentData->second.serial();
	}
}