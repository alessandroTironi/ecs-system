#pragma once

#include <memory>
#include "Types.h"
#include "Entity.h"
#include "IDGenerator.h"

namespace ecs 
{
	class ArchetypesRegistry; 
	class ComponentsRegistry;

	class World : public std::enable_shared_from_this<World>
	{
	public:
		World();
		~World();

		entity_id CreateEntity();

		EntityHandle GetEntity(entity_id id);

	private:
		std::shared_ptr<ArchetypesRegistry> m_archetypesRegistry;
		std::shared_ptr<ComponentsRegistry> m_componentsRegistry;

		IDGenerator<entity_id> m_entityIDGenerator;
	};
}