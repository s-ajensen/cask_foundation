#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/resource/resource_store.hpp>
#include <cask/resource/mesh_data.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/event/event_queue.hpp>
#include <cstring>

struct MeshTestContext : CompactableTestContext {
    ResourceStore<MeshData>* mesh_store() {
        uint32_t store_id = world.register_component("MeshStore");
        return world.get<ResourceStore<MeshData>>(store_id);
    }

    ComponentStore<MeshHandle>* mesh_components() {
        uint32_t components_id = world.register_component("MeshComponents");
        return world.get<ComponentStore<MeshHandle>>(components_id);
    }
};

SCENARIO("mesh plugin reports its metadata", "[mesh]") {
    GIVEN("the mesh plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is mesh") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "mesh") == 0);
        }

        THEN("it defines MeshStore and MeshComponents") {
            REQUIRE(info->defines_count == 2);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "MeshStore") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "MeshComponents") == 0);
        }

        THEN("it requires EntityCompactor") {
            REQUIRE(info->requires_count == 1);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EntityCompactor") == 0);
        }

        THEN("it provides an init function") {
            REQUIRE(info->init_fn != nullptr);
        }

        THEN("it does not provide tick frame or shutdown functions") {
            REQUIRE(info->tick_fn == nullptr);
            REQUIRE(info->frame_fn == nullptr);
            REQUIRE(info->shutdown_fn == nullptr);
        }
    }
}

SCENARIO("mesh plugin initializes MeshStore and MeshComponents", "[mesh]") {
    GIVEN("a world with EntityCompactor and the mesh plugin") {
        MeshTestContext context;
        REQUIRE(context.info->init_fn != nullptr);

        WHEN("init is called") {
            context.init();

            THEN("MeshStore is registered and retrievable") {
                REQUIRE(context.mesh_store() != nullptr);
            }

            THEN("MeshComponents is registered and retrievable") {
                REQUIRE(context.mesh_components() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("mesh plugin wires MeshComponents into EntityCompactor", "[mesh]") {
    GIVEN("an initialized mesh plugin") {
        MeshTestContext context;
        REQUIRE(context.info->init_fn != nullptr);
        context.init();

        WHEN("an entity with a mesh handle is compacted") {
            uint32_t entity = context.table.create();
            context.mesh_components()->insert(entity, MeshHandle{0});

            EventQueue<DestroyEvent> destroy_queue;
            destroy_queue.emit(DestroyEvent{entity});
            destroy_queue.swap();
            context.compactor.compact(destroy_queue);

            THEN("the entity is no longer alive") {
                REQUIRE_FALSE(context.table.alive(entity));
            }

            THEN("the mesh component is removed") {
                auto* components = context.mesh_components();
                REQUIRE_FALSE(components->has(entity));
            }
        }

        context.shutdown();
    }
}

SCENARIO("mesh plugin shutdown allows reinit on fresh world", "[mesh]") {
    GIVEN("an initialized mesh plugin") {
        MeshTestContext context;
        REQUIRE(context.info->init_fn != nullptr);
        context.init();

        REQUIRE(context.mesh_store() != nullptr);
        REQUIRE(context.mesh_components() != nullptr);

        WHEN("shutdown is called and a fresh world is reinitialized") {
            context.shutdown();

            MeshTestContext fresh_context;
            fresh_context.init();

            THEN("MeshStore is available on the fresh world") {
                REQUIRE(fresh_context.mesh_store() != nullptr);
            }

            THEN("MeshComponents is available on the fresh world") {
                REQUIRE(fresh_context.mesh_components() != nullptr);
            }

            fresh_context.shutdown();
        }
    }
}
