#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/texture_data.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_compactor.hpp>

static ResourceStore<TextureData>* texture_store = nullptr;
static ComponentStore<TextureHandle>* texture_components = nullptr;

static void texture_init(WorldHandle handle) {
    cask::WorldView world(handle);

    auto* compactor = world.get<EntityCompactor>(world.register_component("EntityCompactor"));

    texture_store = new ResourceStore<TextureData>();
    uint32_t store_id = world.register_component("TextureStore");
    world.bind(store_id, texture_store);

    texture_components = new ComponentStore<TextureHandle>();
    uint32_t components_id = world.register_component("TextureComponents");
    world.bind(components_id, texture_components);

    compactor->add(texture_components, remove_component<TextureHandle>);
}

static void texture_shutdown(WorldHandle) {
    delete texture_store;
    texture_store = nullptr;
    delete texture_components;
    texture_components = nullptr;
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
    texture_shutdown
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
