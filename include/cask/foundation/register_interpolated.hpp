#pragma once

#include <cask/world.hpp>
#include <cask/ecs/interpolated.hpp>
#include <cask/ecs/frame_advancer.hpp>

namespace cask {

template<typename ValueType>
Interpolated<ValueType>* register_interpolated(WorldView& world, const char* name) {
    auto* interpolated = world.register_component<Interpolated<ValueType>>(name);
    auto* advancer = world.resolve<FrameAdvancer>("FrameAdvancer");
    advancer->add(interpolated, advance_interpolated<ValueType>);
    return interpolated;
}

}
