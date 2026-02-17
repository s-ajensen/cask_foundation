#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>

static EntityTable* entity_table = nullptr;
static EntityCompactor* compactor = nullptr;

static void entity_init(WorldHandle handle) {
    cask::WorldView world(handle);
    entity_table = new EntityTable();
    uint32_t table_id = world.register_component("EntityTable");
    world.bind(table_id, entity_table);

    compactor = new EntityCompactor();
    compactor->table_ = entity_table;
    uint32_t compactor_id = world.register_component("EntityCompactor");
    world.bind(compactor_id, compactor);
}

static void entity_shutdown(WorldHandle) {
    delete compactor;
    compactor = nullptr;
    delete entity_table;
    entity_table = nullptr;
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
    entity_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
