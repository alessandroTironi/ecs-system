#include "ComponentsDatabase.h"

ecs::IDGenerator<ecs::component_id> ecs::ComponentsDatabase::s_componentIDGenerator;
std::unordered_map<size_t, ecs::component_id> ecs::ComponentsDatabase::s_componentsClassMap;

ecs::component_id ecs::ComponentsDatabase::GetComponentID(const type_hash_t componentHash)
{
	auto optionalID = s_componentsClassMap.find(componentHash);
	if (optionalID == s_componentsClassMap.end())
	{
		const component_id newID = s_componentIDGenerator.GenerateNewUniqueID();
		s_componentsClassMap[componentHash] = newID;
		return newID;
	}
	else
	{
		return optionalID->second;
	}
}