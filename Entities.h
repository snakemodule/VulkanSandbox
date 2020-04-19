#pragma once

#include <array>
#include "boost/multi_array.hpp"
#include <memory>

/*





class EntityComponentSystem
{
	using Entity = std::uint64_t;
	using Component = std::uint64_t;
	using System = std::uint64_t;

	struct EntityMetadata
	{
		std::uint16_t section_offset;
		std::uint16_t data_offset;
	};
	std::vector<EntityMetadata> entity_metadata;

	struct ComponentDefinition
	{
		const char* name;
		size_t size;
		uint8_t alignment;
	};

	

	struct Section
	{
		std::vector<Component> components;
		std::vector<std::byte> component_data;
	};
	
	std::vector<Section> sections;

	Entity create_entity(std::vector<Component> components);

	using T = std::uint64_t;
	T* get_component(Component component, const EntityMetadata& metadata)
	{
		Section* section = &sections[metadata.section_offset];

		for (int i = 0; i < section->components.size(); ++i)
		{
			std::byte* head = &section->component_data[metadata.data_offset];

			std::uint64_t component_id = 0;
			memcpy(&component_id, head, 5);
			head += 5;

			if (component_id == component)
			{
				std::uint8_t component_alignment;
				memcpy(&component_alignment, head, 1);
				head += 1;

				// don't read length unless we need to
				std::uint8_t component_length;
				memcpy(&component_length, head, 2);
				head += 2;

				void * p = (void*)head;
				size_t space = section->components.size() - (metadata.data_offset + 5 + 1 + 2);
				return (T*)std::align(component_alignment, component_length, p, space);
			}

		}

		return nullptr;
	}

public:
	
	void begin(EntityComponentSystem* ecs);

	void init(EntityComponentSystem* ecs);

	void free(EntityComponentSystem* ecs);

	void update(EntityComponentSystem* ecs, float dt);

	Component register_component(EntityComponentSystem* ecs,
		const char* name,
		size_t data_size,
		uint8_t alignment,
		ComponentAllocationStrategy allocation = ComponentAllocationStrategy::block);

	System register_system(EntityComponentSystem* ecs,
		const char* name,
		uint8_t stream_count,
		StreamLayout* streams,
		SystemCbSystemUpdate system_update,
		void* user_data = nullptr);

	Entity entity_create(EntityComponentSystem* ecs, size_t component_count, const Component* components,
		EntityComponentDataInit init = nullptr, void* user_data = nullptr);

	void* entity_get_component(EntityComponentSystem* ecs, Entity entity, Component component);
	void entity_get_all_components(EntityComponentSystem* ecs, Entity entity, void** out_components);



	EntityComponentSystem();
	~EntityComponentSystem();
};

enum ComponentAllocationStrategy
{
	block
};

inline uint32_t read_stream_entry(void* stream, uint32_t entry, ecs::Entity* entity, uint8_t component_count, void** components);
inline uint32_t stream_start(void* stream, ecs::Entity* entity, uint8_t component_count, void** component);


struct Transform
{
	float x;
	float y;
	float rotation;
	float scale;
};

*/
