#pragma once

#include <memory>
#include <unordered_map>
#include <typeinfo>
#include <type_traits>
#include <typeindex>
#include "Types.h"
#include "IDGenerator.h"
#include "ArchetypesRegistry.h"
#include "ISystem.h"

namespace ecs 
{ 
	class ComponentsRegistry;
	class EntityHandle;

	class World : public std::enable_shared_from_this<World>
	{
		friend class EntityHandle;

	public:
		World() = default;
		~World();

		/**
		 * @brief Initialize the world. This must be called before using any other method.
		 */
		void Initialize();

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

		std::shared_ptr<ComponentsRegistry> GetComponentsRegistry() const;
		std::shared_ptr<ArchetypesRegistry> GetArchetypesRegistry() const;

		/**
		 * @brief Registers a system for execution in this world.
		 * @tparam SystemType The type of the system to add.
		 * @return The system.
		 */
		template<typename SystemType>
		std::shared_ptr<SystemType> AddSystem()
		{
			static_assert(std::is_base_of<ISystem, SystemType>::value, 
				"SystemType must inherit from ISystem");
			auto system = std::make_shared<SystemType>();
			m_registeredSystems[std::type_index(typeid(SystemType))] = system;
			return system;
		}

		/**
		 * @brief Gets the system of the given type.
		 * @tparam SystemType The type of the system to get.
		 * @return The system if found, nullptr otherwise.
		 * @throw std::out_of_range if the system is not found.
		 */
		template<typename SystemType>
		std::shared_ptr<SystemType> GetSystem() const
		{
			static_assert(std::is_base_of<ISystem, SystemType>::value, 
				"SystemType must inherit from ISystem");

			std::type_index typeIndex = std::type_index(typeid(SystemType));
			return std::static_pointer_cast<SystemType>(m_registeredSystems.at(typeIndex));
		}

		/**
		 * Looks for a system of the given type. Slower than GetSystem(), but guaranteed to not throw any exceptions.
		 * @tparam SystemType The type of the system to look for.
		 * @return The system if found, nullptr otherwise.
		 */
		template<typename SystemType>
		std::shared_ptr<SystemType> FindSystem() noexcept
		{
			static_assert(std::is_base_of<ISystem, SystemType>::value, 
				"SystemType must inherit from ISystem");
				
			std::type_index typeIndex = std::type_index(typeid(SystemType));
			auto optionalSystem = m_registeredSystems.find(typeIndex);
			if (optionalSystem != m_registeredSystems.end())
			{
				return std::static_pointer_cast<SystemType>(optionalSystem->second);
			}

			return nullptr;
		}

		/**
		 * @brief Removes a system from the world.
		 * @tparam SystemType The type of the system to remove.
		 */
		template<typename SystemType>
		void RemoveSystem()
		{
			static_assert(std::is_base_of<ISystem, SystemType>::value, 
				"SystemType must inherit from ISystem");
			std::type_index typeIndex = std::type_index(typeid(SystemType));
			m_registeredSystems.erase(typeIndex);
		}

		/**
		 * @brief Gets the number of registered systems.
		 * @return The number of registered systems.
		 */
		inline size_t GetSystemsCount() const noexcept { return m_registeredSystems.size();}

		/**
		 * @brief Updates the world, executing all the registered systems.
		 * @param deltaTime The time since the last update.
		 */
		void Update(real_t deltaTime);

	private:
		std::shared_ptr<ArchetypesRegistry> m_archetypesRegistry;
		std::shared_ptr<ComponentsRegistry> m_componentsRegistry;

		IDGenerator<entity_id> m_entityIDGenerator;

		std::unordered_map<type_key, std::shared_ptr<ISystem>> m_registeredSystems;
	};
}