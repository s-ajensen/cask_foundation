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
        return static_cast<EntityTable*>(world.resolve("EntityTable"));
    }

    EntityCompactor* compactor() {
        return static_cast<EntityCompactor*>(world.resolve("EntityCompactor"));
    }

    EventQueue<DestroyEntity>* destroy_entity_queue() {
        return static_cast<EventQueue<DestroyEntity>*>(world.resolve("DestroyEntityQueue"));
    }
};

SCENARIO("entity plugin reports its metadata", "[entity]") {
    GIVEN("the entity plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is entity") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "entity") == 0);
        }

        THEN("it defines EntityTable EntityCompactor DestroyEntityQueue and EntityPluginState") {
            REQUIRE(info->defines_count == 4);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "EntityTable") == 0);
            REQUIRE(std::strcmp(info->defines_components[1], "EntityCompactor") == 0);
            REQUIRE(std::strcmp(info->defines_components[2], "DestroyEntityQueue") == 0);
            REQUIRE(std::strcmp(info->defines_components[3], "EntityPluginState") == 0);
        }

        THEN("it requires the EventSwapper component") {
            REQUIRE(info->requires_count == 1);
            REQUIRE(info->requires_components != nullptr);
            REQUIRE(std::strcmp(info->requires_components[0], "EventSwapper") == 0);
        }

        THEN("it provides an init function") {
            REQUIRE(info->init_fn != nullptr);
        }

        THEN("it provides a tick function") {
            REQUIRE(info->tick_fn != nullptr);
        }

        THEN("it does not provide a shutdown function") {
            REQUIRE(info->shutdown_fn == nullptr);
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
            context.destroy_entity_queue()->emit(DestroyEntity{entity});
            context.swapper.swap_all();
            context.tick();

            THEN("the entity is no longer alive") {
                REQUIRE_FALSE(context.entity_table()->alive(entity));
            }

            THEN("the component is removed from the store") {
                REQUIRE_FALSE(test_store.has(entity));
            }
        }

        context.shutdown();
    }
}

SCENARIO("entity plugin tick isolates destruction between worlds", "[entity]") {
    GIVEN("two initialized worlds with an entity each") {
        EntityTestContext world1;
        world1.init();
        ComponentStore<uint32_t> store1;
        world1.compactor()->add(&store1, remove_component<uint32_t>);
        uint32_t entity1 = world1.entity_table()->create();
        store1.insert(entity1, 10);

        EntityTestContext world2;
        world2.init();
        ComponentStore<uint32_t> store2;
        world2.compactor()->add(&store2, remove_component<uint32_t>);
        uint32_t entity2 = world2.entity_table()->create();
        store2.insert(entity2, 20);

        WHEN("a destroy event is emitted and ticked on world1 only") {
            world1.destroy_entity_queue()->emit(DestroyEntity{entity1});
            world1.swapper.swap_all();
            world1.tick();

            THEN("world1 entity is destroyed") {
                REQUIRE_FALSE(world1.entity_table()->alive(entity1));
            }

            THEN("world2 entity is still alive") {
                REQUIRE(world2.entity_table()->alive(entity2));
            }
        }

        world2.shutdown();
        world1.shutdown();
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
