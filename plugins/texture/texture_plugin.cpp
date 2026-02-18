#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/texture_data.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_compactor.hpp>

static void texture_init(WorldHandle handle) {
    cask::WorldView world(handle);
    auto* compactor = world.resolve<EntityCompactor>("EntityCompactor");
    world.register_component<ResourceStore<TextureData>>("TextureStore");
    auto* texture_components = world.register_component<ComponentStore<TextureHandle>>("TextureComponents");
    compactor->add(texture_components, remove_component<TextureHandle>);
}

static const char* defined_components[] = {"TextureStore", "TextureComponents"};
static const char* required_components[] = {"EntityCompactor"};

static PluginInfo plugin_info = {
    "texture",
    defined_components,
    required_components,
    2,
    1,
    texture_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
