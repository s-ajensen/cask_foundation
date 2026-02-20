#include <catch2/catch_test_macros.hpp>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/event/event_queue.hpp>
#include <cask/foundation/register_component_store.hpp>

struct TestComponent {
    int value;
};

SCENARIO("registering a component store makes it resolvable from the world", "[registration]") {
    GIVEN("a world with an EntityCompactor") {
        World world;
        WorldHandle handle = handle_from_world(&world);
        cask::WorldView view(handle);

        auto* compactor = view.register_component<EntityCompactor>("EntityCompactor");
        EntityTable table;
        compactor->table_ = &table;

        WHEN("register_component_store is called") {
            auto* store = cask::register_component_store<TestComponent>(view, "TestComponents");

            THEN("the store is not null") {
                REQUIRE(store != nullptr);
            }

            THEN("the store is resolvable by name") {
                auto* resolved = view.resolve<ComponentStore<TestComponent>>("TestComponents");
                REQUIRE(resolved == store);
            }
        }
    }
}

SCENARIO("registered component store is wired to the EntityCompactor", "[registration]") {
    GIVEN("a world with an EntityCompactor and a registered component store") {
        World world;
        WorldHandle handle = handle_from_world(&world);
        cask::WorldView view(handle);

        auto* compactor = view.register_component<EntityCompactor>("EntityCompactor");
        EntityTable table;
        compactor->table_ = &table;
        auto* store = cask::register_component_store<TestComponent>(view, "TestComponents");

        WHEN("an entity with a component is compacted") {
            uint32_t entity = table.create();
            store->insert(entity, TestComponent{99});

            EventQueue<DestroyEntity> destroy_queue;
            destroy_queue.emit(DestroyEntity{entity});
            destroy_queue.swap();
            compactor->compact(destroy_queue);

            THEN("the entity is no longer alive") {
                REQUIRE_FALSE(table.alive(entity));
            }

            THEN("the component is removed from the store") {
                REQUIRE(store->entity_to_index_.find(entity) == store->entity_to_index_.end());
            }
        }
    }
}
