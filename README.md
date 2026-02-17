# cask_foundation

Reusable foundational plugins for the [cask](https://github.com/s-ajensen/cask_engine) game engine. These plugins extract common infrastructure — entities, events, interpolation, and resources — into shared libraries that game plugins declare dependencies on via the engine's `defines_components` / `requires_components` mechanism.

## Plugins

| Plugin | Defines | Requires | Tick Behavior |
|--------|---------|----------|---------------|
| `event_plugin` | EventSwapper | — | Calls `swap_all()` on all registered event queues |
| `interpolation_plugin` | FrameAdvancer | — | Calls `advance_all()` on all registered interpolated values |
| `resource_plugin` | MeshStore, TextureStore | — | — |
| `entity_plugin` | EntityTable, EntityCompactor | EventSwapper | — |

### Dependency Graph

```
event_plugin          (no dependencies)
interpolation_plugin  (no dependencies)
resource_plugin       (no dependencies)
entity_plugin         (requires: event_plugin)
```

The engine's dependency graph ensures `event_plugin` loads before `entity_plugin`. Interpolation and resource plugins have no dependencies and can load in any order.

## Usage

With these plugins loaded, a game plugin can skip all infrastructure setup and focus on game logic:

```cpp
void game_init(WorldHandle handle) {
    cask::WorldView world(handle);

    // Foundation plugins already allocated and bound these — just retrieve them
    auto* entities  = world.get<EntityTable>(world.register_component("EntityTable"));
    auto* compactor = world.get<EntityCompactor>(world.register_component("EntityCompactor"));
    auto* swapper   = world.get<EventSwapper>(world.register_component("EventSwapper"));
    auto* meshes    = world.get<ResourceStore<MeshData>>(world.register_component("MeshStore"));
    auto* textures  = world.get<ResourceStore<TextureData>>(world.register_component("TextureStore"));

    // Then define only game-specific components and wire them in
    auto* spawn_queue = new EventQueue<SpawnEvent>();
    swapper->add(spawn_queue, swap_queue<SpawnEvent>);

    auto* transform_store = new ComponentStore<Transform>();
    compactor->add(transform_store, remove_component<Transform>);
}
```

## Building

```bash
cmake -B build
cmake --build build
```

## Testing

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

## Dependencies

Fetched automatically via CMake FetchContent:

- [cask_engine](https://github.com/s-ajensen/cask_engine) — Engine ABI and runtime
- [cask_core](https://github.com/s-ajensen/cask_core) — ECS, events, interpolation, and resource headers
- [Catch2 v3](https://github.com/catchorg/Catch2) — Testing (test targets only)
