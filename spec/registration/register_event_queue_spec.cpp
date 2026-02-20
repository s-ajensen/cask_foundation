#include <catch2/catch_test_macros.hpp>
#include <cask/world/world.hpp>
#include <cask/world/abi_internal.hpp>
#include <cask/world.hpp>
#include <cask/event/event_swapper.hpp>
#include <cask/event/event_queue.hpp>
#include <cask/foundation/register_event_queue.hpp>

struct TestEvent {
    int value;
};

SCENARIO("registering an event queue makes it resolvable from the world", "[registration]") {
    GIVEN("a world with an EventSwapper") {
        World world;
        WorldHandle handle = handle_from_world(&world);
        cask::WorldView view(handle);

        auto* swapper = view.register_component<EventSwapper>("EventSwapper");

        WHEN("register_event_queue is called") {
            auto* queue = cask::register_event_queue<TestEvent>(view, "TestEventQueue");

            THEN("the queue is not null") {
                REQUIRE(queue != nullptr);
            }

            THEN("the queue is resolvable by name") {
                auto* resolved = view.resolve<EventQueue<TestEvent>>("TestEventQueue");
                REQUIRE(resolved == queue);
            }
        }
    }
}

SCENARIO("registered event queue is wired to the EventSwapper", "[registration]") {
    GIVEN("a world with an EventSwapper and a registered event queue") {
        World world;
        WorldHandle handle = handle_from_world(&world);
        cask::WorldView view(handle);

        auto* swapper = view.register_component<EventSwapper>("EventSwapper");
        auto* queue = cask::register_event_queue<TestEvent>(view, "TestEventQueue");

        WHEN("an event is emitted and the swapper swaps all") {
            queue->emit(TestEvent{42});
            swapper->swap_all();

            THEN("the event is available via poll") {
                auto& events = queue->poll();
                REQUIRE(events.size() == 1);
                REQUIRE(events[0].value == 42);
            }
        }
    }
}
