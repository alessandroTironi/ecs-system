#pragma once

#include <memory>
#include "Types.h"
#include "IDGenerator.h"

namespace ecs 
{ 
	class ComponentsRegistry;
	class EntityHandle;
	class ArchetypesRegistry;

	class World : public std::enable_shared_from_this<World>
	{
		friend class EntityHandle;

	public:
		World();
		~World();

		entity_id CreateEntity();

		EntityHandle GetEntity(entity_id id);

	protected:
		ComponentsRegistry* GetComponentsRegistry() const;
		ArchetypesRegistry* GetArchetypesRegistry() const;

	private:
		std::shared_ptr<ArchetypesRegistry> m_archetypesRegistry;
		std::shared_ptr<ComponentsRegistry> m_componentsRegistry;

		IDGenerator<entity_id> m_entityIDGenerator;
	};
}