#pragma once

#include <cask/world.hpp>
#include <cask/event/event_queue.hpp>
#include <cask/event/event_swapper.hpp>

namespace cask {

template<typename Event>
EventQueue<Event>* register_event_queue(WorldView& world, const char* name) {
    auto* queue = world.register_component<EventQueue<Event>>(name);
    auto* swapper = world.resolve<EventSwapper>("EventSwapper");
    swapper->add(queue, swap_queue<Event>);
    return queue;
}

}
