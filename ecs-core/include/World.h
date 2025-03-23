#pragma once

#include <memory>
#include "Types.h"
#include "IDGenerator.h"
#include "ArchetypesRegistry.h"

namespace ecs 
{ 
	class ComponentsRegistry;
	class EntityHandle;

	class World : public std::enable_shared_from_this<World>
	{
		friend class EntityHandle;

	public:
		World();
		~World();

		/**
		 * @brief Create an entity with no components.
		 * @return The entity ID
		 */
		entity_id CreateEntity();

		/**
		 * @brief Create an entity with at least one component.
		 * @tparam FirstComponent The first component type
		 * @tparam OtherComponents The other component types
		 * @return The entity ID
		 */
		template<typename FirstComponent, typename... OtherComponents>
		entity_id CreateEntity()
		{
			const entity_id id = m_entityIDGenerator.GenerateNewUniqueID();
			m_archetypesRegistry->AddEntity<FirstComponent, OtherComponents...>(id);
			return id;
		}

		/**
		 * @brief 	Creates a handle for the entity with the given ID. 
		 * 			A handle is a lightweight object that allows to access to utility APIs 
		 * 			allowing entity operations like components management.
		 * @param id The entity ID
		 * @return The entity handle
		 */
		EntityHandle GetEntity(entity_id id);

		ComponentsRegistry* GetComponentsRegistry() const;
		ArchetypesRegistry* GetArchetypesRegistry() const;

	private:
		std::shared_ptr<ArchetypesRegistry> m_archetypesRegistry;
		std::shared_ptr<ComponentsRegistry> m_componentsRegistry;

		IDGenerator<entity_id> m_entityIDGenerator;
	};
}