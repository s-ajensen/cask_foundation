#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/event/event_swapper.hpp>
#include <cask/event/event_queue.hpp>
#include <cstring>

struct EntityTestContext : PluginTestContext {
    EventSwapper swapper;

    EntityTestContext() {
        uint32_t swapper_id = world.register_component("EventSwapper");
        world.bind(swapper_id, &swapper);
    }

    EntityTable* entity_table() {
        uint32_t table_id = world.register_component("EntityTable");
        return world.get<EntityTable>(table_id);
    }

    EntityCompactor* compactor() {
        uint32_t compactor_id = world.register_component("EntityCompactor");
        return world.get<EntityCompactor>(compactor_id);
    }

    EventQueue<DestroyEntity>* destroy_entity_queue() {
        uint32_t queue_id = world.register_component("DestroyEntityQueue");
        return world.get<EventQueue<DestroyEntity>>(queue_id);
    }
};

SCENARIO("entity plugin reports its metadata", "[entity]") {
    GIVEN("the entity plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is entity") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "entity") == 0);
        }

        THEN("it defines EntityTable EntityCompactor and DestroyEntityQueue") {
            REQUIRE(info->defines_count == 3);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "EntityTable") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "EntityCompactor") == 0);
            REQUIRE(std::strcmp(info->defines_components[2], "DestroyEntityQueue") == 0);
        }

        THEN("it requires the EventSwapper component") {
            REQUIRE(info->requires_count == 1);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EventSwapper") == 0);
        }

        THEN("it provides an init function") {
            REQUIRE(info->init_fn != nullptr);
        }

        THEN("it provides tick and shutdown functions") {
            REQUIRE(info->tick_fn != nullptr);
            REQUIRE(info->shutdown_fn != nullptr);
        }

        THEN("it does not provide a frame function") {
            REQUIRE(info->frame_fn == nullptr);
        }
    }
}

SCENARIO("entity plugin initializes EntityTable and EntityCompactor", "[entity]") {
    GIVEN("a world and the entity plugin") {
        EntityTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("EntityTable is registered and retrievable") {
                REQUIRE(context.entity_table() != nullptr);
            }

            THEN("EntityCompactor is registered and retrievable") {
                REQUIRE(context.compactor() != nullptr);
            }

            THEN("the compactor table pointer is set to the entity table") {
                REQUIRE(context.compactor()->table_ == context.entity_table());
            }

            context.shutdown();
        }
    }
}

SCENARIO("entity plugin initializes DestroyEntityQueue", "[entity]") {
    GIVEN("a world and the entity plugin") {
        EntityTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("DestroyEntityQueue is registered and retrievable") {
                REQUIRE(context.destroy_entity_queue() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("entity plugin tick compacts destroyed entities", "[entity]") {
    GIVEN("an initialized entity plugin with a component store") {
        EntityTestContext context;
        context.init();

        ComponentStore<uint32_t> test_store;
        context.compactor()->add(&test_store, remove_component<uint32_t>);

        uint32_t entity = context.entity_table()->create();
        test_store.insert(entity, 42);

        WHEN("a DestroyEntity event is emitted and tick is called") {
            auto* queue = context.destroy_entity_queue();
            REQUIRE(queue != nullptr);
            queue->emit(DestroyEntity{entity});
            context.swapper.swap_all();
            REQUIRE(context.info->tick_fn != nullptr);
            context.tick();

            THEN("the entity is no longer alive") {
                REQUIRE_FALSE(context.entity_table()->alive(entity));
            }

            THEN("the component is removed from the store") {
                REQUIRE(test_store.entity_to_index_.find(entity) == test_store.entity_to_index_.end());
            }
        }

        context.shutdown();
    }
}

SCENARIO("entity plugin shutdown allows reinit on fresh world", "[entity]") {
    GIVEN("an initialized entity plugin") {
        EntityTestContext context;
        context.init();

        REQUIRE(context.entity_table() != nullptr);
        REQUIRE(context.compactor() != nullptr);

        WHEN("shutdown is called and a fresh world is reinitialized") {
            context.shutdown();

            EntityTestContext fresh_context;
            fresh_context.init();

            THEN("EntityTable is available on the fresh world") {
                REQUIRE(fresh_context.entity_table() != nullptr);
            }

            THEN("EntityCompactor is available on the fresh world") {
                REQUIRE(fresh_context.compactor() != nullptr);
            }

            fresh_context.shutdown();
        }
    }
}
