#pragma once

#include <cask/abi.h>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>

extern "C" PluginInfo* get_plugin_info();

struct PluginTestContext {
    World world;
    WorldHandle handle;
    PluginInfo* info;

    PluginTestContext()
        : handle(handle_from_world(&world))
        , info(get_plugin_info()) {}

    void init() { info->init_fn(handle); }
    void tick() { info->tick_fn(handle); }
    void shutdown() {
        if (info->shutdown_fn) info->shutdown_fn(handle);
        for (size_t index = 0; index < info->defines_count; ++index) {
            world.destroy(info->defines_components[index]);
        }
    }
};

struct DestroyEvent {
    uint32_t entity;
};

struct CompactableTestContext : PluginTestContext {
    EntityTable table;
    EntityCompactor compactor;

    CompactableTestContext() {
        compactor.table_ = &table;
        uint32_t table_id = world.register_component("EntityTable");
        world.bind(table_id, &table);
        uint32_t compactor_id = world.register_component("EntityCompactor");
        world.bind(compactor_id, &compactor);
    }
};
