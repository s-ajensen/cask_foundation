#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>

static void entity_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* table = world.register_component<EntityTable>("EntityTable");
    auto* compactor = world.register_component<EntityCompactor>("EntityCompactor");
    compactor->table_ = table;
}

static const char* defined_components[] = {"EntityTable", "EntityCompactor"};
static const char* required_components[] = {"EventSwapper"};

static PluginInfo plugin_info = {
    "entity",
    defined_components,
    required_components,
    2,
    1,
    entity_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
