#pragma once

#include <cask/world.hpp>
#include <cask/resource/resource_sources.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/resource_loader_registry.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe_resource_sources.hpp>
#include <cask/schema/describe_resource_components.hpp>

namespace cask {

template<typename Resource>
ResourceSources<Resource>* register_serializable_resource(
    WorldView& world,
    const char* store_name,
    const char* components_name,
    const char* sources_name,
    const char* loader_registry_name) {
    auto* registry = world.resolve<SerializationRegistry>("SerializationRegistry");
    auto* store = world.resolve<ResourceStore<Resource>>(store_name);
    auto* loader_registry = world.resolve<ResourceLoaderRegistry<Resource>>(loader_registry_name);
    auto* sources = world.register_component<ResourceSources<Resource>>(sources_name);

    auto sources_entry = describe_resource_sources<Resource>(sources_name, *store, *loader_registry);
    auto components_entry = describe_resource_components<Resource>(components_name, sources_name, *store);

    registry->add(sources_name, sources_entry);
    registry->add(components_name, components_entry);

    return sources;
}

}
