#pragma once

#include <cask/world.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe_component_store.hpp>
#include <string>

namespace cask {

template<typename T>
ComponentStore<T>* register_serializable_store(WorldView& world, const char* name, const RegistryEntry& value_entry) {
    auto* compactor = world.resolve<EntityCompactor>("EntityCompactor");
    auto* store = world.register_component<ComponentStore<T>>(name);
    compactor->add(store, remove_component<T>);

    auto store_entry = describe_component_store<T>(name, value_entry);
    auto* registry = world.resolve<SerializationRegistry>("SerializationRegistry");

    std::string value_name = value_entry.schema["name"];
    registry->add(value_name, value_entry);
    registry->add(name, store_entry);

    return store;
}

}
