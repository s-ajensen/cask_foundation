#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe_entity_registry.hpp>
#include <cask/ecs/entity_table.hpp>

struct SerializationPluginState {
    cask::SerializationRegistry* serialization_registry;
};

static void serialization_init(WorldHandle handle) {
    cask::WorldView world(handle);

    auto* state = world.register_component<SerializationPluginState>("SerializationPluginState");
    state->serialization_registry = world.register_component<cask::SerializationRegistry>("SerializationRegistry");

    auto* entity_table = world.resolve<EntityTable>("EntityTable");
    auto entity_registry_entry = cask::describe_entity_registry("EntityRegistry", *entity_table);
    state->serialization_registry->add("EntityRegistry", std::move(entity_registry_entry));
}

static const char* defined_components[] = {"SerializationRegistry", "SerializationPluginState"};
static const char* required_components[] = {"EntityTable", "EntityRegistry"};

static PluginInfo plugin_info = {
    "serialization",
    defined_components,
    required_components,
    2,
    2,
    serialization_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
