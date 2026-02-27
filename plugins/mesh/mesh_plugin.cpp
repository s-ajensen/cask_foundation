#include <cask/abi.h>
#include <cask/world.hpp>
#include <cask/resource/resource_store.hpp>
#include <cask/resource/resource_descriptor.hpp>
#include <cask/resource/mesh_data.hpp>
#include <cask/resource/resource_loader_registry.hpp>
#include <cask/foundation/register_component_store.hpp>

static void mesh_init(WorldHandle handle) {
    cask::WorldView world(handle);
    world.register_component<ResourceStore<MeshData>>(ResourceDescriptor<MeshData>::store);
    cask::register_component_store<MeshHandle>(world, ResourceDescriptor<MeshData>::components);
    world.register_component<cask::ResourceLoaderRegistry<MeshData>>(ResourceDescriptor<MeshData>::loader_registry);
}

static const char* defined_components[] = {
    ResourceDescriptor<MeshData>::store,
    ResourceDescriptor<MeshData>::components,
    ResourceDescriptor<MeshData>::loader_registry
};
static const char* required_components[] = {"EntityCompactor"};

static PluginInfo plugin_info = {
    "mesh",
    defined_components,
    required_components,
    3,
    1,
    mesh_init,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PluginInfo* get_plugin_info() {
    return &plugin_info;
}
