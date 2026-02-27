#pragma once

#include <cask/world.hpp>
#include <cask/resource/resource_descriptor.hpp>
#include <cask/resource/resource_sources.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/resource_loader_registry.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe_resource_sources.hpp>
#include <cask/schema/describe_resource_components.hpp>

namespace cask {

template<typename Resource>
ResourceSources<Resource>* register_serializable_resource(WorldView& world) {
    auto* registry = world.resolve<SerializationRegistry>("SerializationRegistry");
    auto* store = world.resolve<ResourceStore<Resource>>(ResourceDescriptor<Resource>::store);
    auto* loader_registry = world.resolve<ResourceLoaderRegistry<Resource>>(ResourceDescriptor<Resource>::loader_registry);
    auto* sources = world.register_component<ResourceSources<Resource>>(ResourceDescriptor<Resource>::sources);

    auto sources_entry = describe_resource_sources<Resource>(ResourceDescriptor<Resource>::sources, *store, *loader_registry);
    auto components_entry = describe_resource_components<Resource>(ResourceDescriptor<Resource>::components, ResourceDescriptor<Resource>::sources, *store);

    registry->add(ResourceDescriptor<Resource>::sources, sources_entry);
    registry->add(ResourceDescriptor<Resource>::components, components_entry);

    return sources;
}

}
