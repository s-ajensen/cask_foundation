#pragma once

#include <cask/abi.h>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>

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
    void shutdown() { info->shutdown_fn(handle); }
};
