#include <catch2/catch_test_macros.hpp>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>
#include <cask/world.hpp>
#include <cask/ecs/entity_table.hpp>
#include <cask/ecs/entity_compactor.hpp>
#include <cask/ecs/component_store.hpp>
#include <cask/ecs/entity_events.hpp>
#include <cask/event/event_queue.hpp>
#include <cask/event/event_swapper.hpp>
#include <cask/schema/serialization_registry.hpp>
#include <cask/schema/describe.hpp>
#include <cask/schema/cask_component.hpp>
#include <cask/foundation/register_serializable_store.hpp>

CASK_COMPONENT(TestValue,
    (float, health),
    (int32_t, score)
)

struct SerializableStoreContext {
    World world;
    WorldHandle handle;
    cask::WorldView view;
    EntityTable table;

    SerializableStoreContext()
        : handle(handle_from_world(&world))
        , view(handle) {
        auto* compactor = view.register_component<EntityCompactor>("EntityCompactor");
        compactor->table_ = &table;

        uint32_t table_id = view.register_component("EntityTable");
        world.bind(table_id, &table);

        view.register_component<EventSwapper>("EventSwapper");
        view.register_component<cask::SerializationRegistry>("SerializationRegistry");
    }

    cask::SerializationRegistry* registry() {
        return view.resolve<cask::SerializationRegistry>("SerializationRegistry");
    }
};

SCENARIO("registering a serializable store makes it resolvable from the world", "[registration]") {
    GIVEN("a world with required infrastructure") {
        SerializableStoreContext context;
        auto value_entry = TestValue::describe();

        WHEN("register_serializable_store is called") {
            auto* store = cask::register_serializable_store<TestValue>(
                context.view, "TestValueComponents", value_entry);

            THEN("the store is not null") {
                REQUIRE(store != nullptr);
            }

            THEN("the store is resolvable by name") {
                auto* resolved = context.view.resolve<ComponentStore<TestValue>>("TestValueComponents");
                REQUIRE(resolved == store);
            }
        }
    }
}

SCENARIO("serializable store is wired to the EntityCompactor", "[registration]") {
    GIVEN("a world with a registered serializable store") {
        SerializableStoreContext context;
        auto value_entry = TestValue::describe();
        auto* store = cask::register_serializable_store<TestValue>(
            context.view, "TestValueComponents", value_entry);

        WHEN("an entity with a component is compacted") {
            uint32_t entity = context.table.create();
            store->insert(entity, TestValue{100.0f, 42});

            auto* compactor = context.view.resolve<EntityCompactor>("EntityCompactor");
            EventQueue<DestroyEntity> destroy_queue;
            destroy_queue.emit(DestroyEntity{entity});
            destroy_queue.swap();
            compactor->compact(destroy_queue);

            THEN("the component is removed from the store") {
                REQUIRE(store->entity_to_index_.find(entity) == store->entity_to_index_.end());
            }
        }
    }
}

SCENARIO("serializable store registers value type in SerializationRegistry", "[registration]") {
    GIVEN("a world with required infrastructure") {
        SerializableStoreContext context;
        auto value_entry = TestValue::describe();

        WHEN("register_serializable_store is called") {
            cask::register_serializable_store<TestValue>(
                context.view, "TestValueComponents", value_entry);

            THEN("the value type is registered in the SerializationRegistry") {
                REQUIRE(context.registry()->has("TestValue"));
            }
        }
    }
}

SCENARIO("serializable store registers store in SerializationRegistry", "[registration]") {
    GIVEN("a world with required infrastructure") {
        SerializableStoreContext context;
        auto value_entry = TestValue::describe();

        WHEN("register_serializable_store is called") {
            cask::register_serializable_store<TestValue>(
                context.view, "TestValueComponents", value_entry);

            THEN("the store is registered in the SerializationRegistry") {
                REQUIRE(context.registry()->has("TestValueComponents"));
            }
        }
    }
}

SCENARIO("serializable store entry has correct dependencies", "[registration]") {
    GIVEN("a world with a registered serializable store") {
        SerializableStoreContext context;
        auto value_entry = TestValue::describe();
        cask::register_serializable_store<TestValue>(
            context.view, "TestValueComponents", value_entry);

        WHEN("the store entry is inspected") {
            auto& entry = context.registry()->get("TestValueComponents");

            THEN("its dependencies include EntityRegistry") {
                REQUIRE(entry.dependencies.size() == 1);
                REQUIRE(entry.dependencies[0] == "EntityRegistry");
            }
        }
    }
}
