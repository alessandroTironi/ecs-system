# ECS System

An Entity Component System (ECS) written in C++ 20 for fun and learning purposes. 

This system is not intended for use in professional projects and it's still under development. 

## Content

This CMake project contains three subprojects exposed below. Any external dependency (google test, SDL) are automatically downloaded during the cmake build process.

- **ecs-core**: the core of the ECS implementation. It is compiled as a static library that must be linked to any project meant to make use of it.
- **ecs-test**: a suite of unit tests implemented with the [gtest](https://github.com/google/googletest) framework. 
- **ecs-runner**: an SDL application that showcases the basic functionalities of the ECS system, spawning 20000 particles launched in random directions and bouncing on the borders of the window.

## Getting started

The project uses CMake as building framework. 

```
cd path/to/project
mkdir build
cd build

cmake ..

cmake --build .
```

Heart of the system is the ecs::World class, from which you can have access to all the core functionalities of an Entity Component System.

```cpp

// Create and initialize World instance
std::shared_ptr<ecs::World> world = std::make_shared<ecs::World>();
world->Initialize();

// Create an entity and store the entity ID into entity
const ecs::entity_id entity = world->CreateEntity<comps::Velocity, comps::Rect, comps::Color>();

// Gets a handle to the entity (essential for accessing methods required for components manipulation)
ecs::EntityHandle handle = world->GetEntity(entity);

// Modify component's data
comps::Velocity& velocity = handle.GetComponent<comps::Velocity>();
velocity.x = generateRandomVelocity();
velocity.y = generateRandomVelocity();

// Add systems
world->AddSystem<systems::MovementSystem>();

// run the system
while (s_keepUpdating)
{
	world->Update(deltaTime);
}

```

Example of a system:

```cpp

class MovementSystem : public ecs::ISystem 
{
	void Update(std::weak_ptr<ecs::World> world, ecs::real_t deltaTime) override
	{
		ecs::query<comps::Rect, comps::Velocity>::MakeQuery(world).forEach(
			[deltaTime](ecs::EntityHandle entity, comps::Rect& rect, comps::Velocity& velocity)
			{
				rect.rect.x += static_cast<int>(velocity.x * deltaTime);
				rect.rect.y += static_cast<int>(velocity.y * deltaTime);

				// Simple bouncing off the edges of the screen
				if (rect.rect.x < 0 || rect.rect.x > SCREEN_WIDTH)
				{
					velocity.x *= -1.0f;
					if (rect.rect.x < 0)
					{
						rect.rect.x = 3;
					}
					else
					{
						rect.rect.x = SCREEN_WIDTH - 3;
					}
				}

				if (rect.rect.y < 0 || rect.rect.y > SCREEN_HEIGHT)
				{
					velocity.y *= -1.0f;
					if (rect.rect.y < 0)
					{
						rect.rect.y = 3;
					}
					else
					{
						rect.rect.y = SCREEN_HEIGHT - 3;
					}
				}
			});
	}
};

```
