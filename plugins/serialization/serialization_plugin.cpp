#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe_entity_registry.hpp>
#include <cask/identity/entity_registry.hpp>
#include <cask/ecs/entity_table.hpp>

static cask::SerializationRegistry* serialization_registry = nullptr;

static void serialization_init(WorldHandle handle) {
    cask::WorldView world(handle);

    serialization_registry = world.register_component<cask::SerializationRegistry>("SerializationRegistry");

    auto* entity_table = world.resolve<EntityTable>("EntityTable");
    auto* entity_registry = world.resolve<EntityRegistry>("EntityRegistry");

    auto entity_registry_entry = cask::describe_entity_registry("EntityRegistry", *entity_table);
    serialization_registry->add("EntityRegistry", std::move(entity_registry_entry));
}

static void serialization_shutdown(WorldHandle) {
    serialization_registry = nullptr;
}

static const char* defined_components[] = {"SerializationRegistry"};
static const char* required_components[] = {"EntityTable", "EntityRegistry"};

static PluginInfo plugin_info = {
    "serialization",
    defined_components,
    required_components,
    1,
    2,
    serialization_init,
    nullptr,
    nullptr,
    serialization_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
