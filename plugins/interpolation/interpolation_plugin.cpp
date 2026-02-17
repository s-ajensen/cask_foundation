#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/ecs/frame_advancer.hpp>

static FrameAdvancer* advancer = nullptr;

static void interpolation_init(WorldHandle handle) {
    cask::WorldView world(handle);
    advancer = new FrameAdvancer();
    uint32_t advancer_id = world.register_component("FrameAdvancer");
    world.bind(advancer_id, advancer);
}

static void interpolation_tick(WorldHandle) {
    if (!advancer) return;
    advancer->advance_all();
}

static void interpolation_shutdown(WorldHandle) {
    delete advancer;
    advancer = nullptr;
}

static const char* defined_components[] = {"FrameAdvancer"};

static PluginInfo plugin_info = {
    "interpolation",
    defined_components,
    nullptr,
    1,
    0,
    interpolation_init,
    interpolation_tick,
    nullptr,
    interpolation_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
