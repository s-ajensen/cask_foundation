#include <catch2/catch_test_macros.hpp>
#include "../plugin_test_context.hpp"
#include <cask/event/event_swapper.hpp>
#include <cask/event/event_queue.hpp>
#include <cstring>

struct TestEvent {
    int value;
};

struct EventTestContext : PluginTestContext {
    EventSwapper* swapper() {
        uint32_t swapper_id = world.register_component("EventSwapper");
        return world.get<EventSwapper>(swapper_id);
    }
};

SCENARIO("event plugin reports its metadata", "[event]") {
    GIVEN("the event plugin") {
        PluginInfo* info = get_plugin_info();

        THEN("the plugin name is event") {
            REQUIRE(info->name != nullptr);
            REQUIRE(std::strcmp(info->name, "event") == 0);
        }

        THEN("it defines the EventSwapper component") {
            REQUIRE(info->defines_count == 1);
            REQUIRE(info->defines_components != nullptr);
            REQUIRE(std::strcmp(info->defines_components[0], "EventSwapper") == 0);
        }

        THEN("it requires no components") {
            REQUIRE(info->requires_count == 0);
            REQUIRE(info->requires_components == nullptr);
        }

        THEN("it provides init tick and shutdown functions") {
            REQUIRE(info->init_fn != nullptr);
            REQUIRE(info->tick_fn != nullptr);
            REQUIRE(info->shutdown_fn != nullptr);
        }

        THEN("it does not provide a frame function") {
            REQUIRE(info->frame_fn == nullptr);
        }
    }
}

SCENARIO("event plugin initializes EventSwapper", "[event]") {
    GIVEN("a world and the event plugin") {
        EventTestContext context;

        WHEN("init is called") {
            context.init();

            THEN("EventSwapper is registered and retrievable") {
                REQUIRE(context.swapper() != nullptr);
            }

            context.shutdown();
        }
    }
}

SCENARIO("event plugin tick swaps all event queues", "[event]") {
    GIVEN("an initialized event plugin with a registered event queue") {
        EventTestContext context;
        context.init();

        EventQueue<TestEvent> queue;
        context.swapper()->add(&queue, swap_queue<TestEvent>);

        WHEN("an event is emitted and tick is called") {
            queue.emit(TestEvent{42});
            context.tick();

            THEN("the event is available via poll") {
                auto& events = queue.poll();
                REQUIRE(events.size() == 1);
                REQUIRE(events[0].value == 42);
            }
        }

        context.shutdown();
    }
}

SCENARIO("event plugin shutdown prevents tick from processing events", "[event]") {
    GIVEN("an initialized event plugin with a pending event") {
        EventTestContext context;
        context.init();

        EventQueue<TestEvent> queue;
        context.swapper()->add(&queue, swap_queue<TestEvent>);
        queue.emit(TestEvent{7});

        WHEN("shutdown is called and then tick is called") {
            context.shutdown();
            context.tick();

            THEN("the pending event is not swapped") {
                REQUIRE(queue.poll().empty());
            }
        }
    }
}
