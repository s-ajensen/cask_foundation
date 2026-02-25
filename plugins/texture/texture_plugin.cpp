#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/resource_loader_registry.hpp>
#include <cask/resource/texture_data.hpp>
#include <cask/foundation/register_component_store.hpp>

static void texture_init(WorldHandle handle) {
    cask::WorldView world(handle);
    world.register_component<ResourceStore<TextureData>>("TextureStore");
    cask::register_component_store<TextureHandle>(world, "TextureComponents");
    world.register_component<cask::ResourceLoaderRegistry<TextureData>>("TextureLoaderRegistry");
}

static const char* defined_components[] = {"TextureStore", "TextureComponents", "TextureLoaderRegistry"};
static const char* required_components[] = {"EntityCompactor"};

static PluginInfo plugin_info = {
    "texture",
    defined_components,
    required_components,
    3,
    1,
    texture_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
