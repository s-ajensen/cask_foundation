---
Handoff: Build cask_foundation
Context
cask_foundation is a new project that compiles reusable foundational plugins from cask_core's APIs. Currently, every plugin that uses entities, events, or resources must allocate, register, bind, and manage these systems from scratch (see cask_example/src/teapot_visual.cpp lines 322-361 — pure boilerplate). cask_foundation extracts this into shared plugins that game plugins declare dependencies on via the engine's defines_components / requires_components mechanism.
Project Setup
Location: /Users/sajensen/code/games/cask_foundation
Dependencies: cask_engine (ABI, via FetchContent from git@github.com:s-ajensen/cask_engine.git) and cask_core (headers, via FetchContent from git@github.com:s-ajensen/cask_core.git).
Build artifacts: Compiled shared libraries (.dylib on macOS). Each plugin is a separate .cpp file producing a separate shared library. Each plugin exports extern "C" PluginInfo* get_plugin_info() per the ABI in cask/abi.h.
CMakeLists.txt pattern: Follow cask_example/CMakeLists.txt as a reference for FetchContent setup and shared library targets. Each plugin target links against cask_engine and cask_core. Set PREFIX "" on each plugin target so the output is entity_plugin.dylib, not libentity_plugin.dylib.
Testing: Each plugin needs a Catch2 BDD-style test spec. The test approach is: instantiate an Engine (from cask/engine/engine.hpp), add the plugin's system to it, step the engine with a fake clock, and verify that components are correctly registered, bound, and managed. Use the same FakeClock pattern from cask_engine's test suite. Load the C++ skill for conventions.
Plugin Specifications
1. Entity Plugin (entity_plugin.cpp)
Defines components: "EntityTable", "EntityCompactor"
Requires components: "EventSwapper" (needs EventSwapper to exist, since EntityCompactor's lifecycle is tied to events)
init_fn:
- Allocates EntityTable and EntityCompactor
- Registers and binds both to the world
- Sets compactor->table_ to point at the entity table
tick_fn:
- No tick behavior of its own. EntityCompactor::compact() is templated on event type and needs a specific destroy event queue — it can't be called generically here. Game plugins call compactor->compact(their_destroy_queue) in their own tick. The entity plugin just owns and provides the infrastructure.
- Important design note: EntityCompactor's compact() method is templated on the event type (template<typename Event> void compact(EventQueue<Event>& queue)). This means the entity plugin cannot call compact itself — it doesn't know the event type. The entity plugin's tick_fn should be nullptr. Game plugins that define destroy events call compactor->compact() in their own tick.
frame_fn: nullptr
shutdown_fn:
- Deletes EntityTable and EntityCompactor
Notes: Game plugins that define entity-bound ComponentStores still need to register those stores with the compactor via compactor->add(store, remove_component<T>). The entity plugin provides the compactor; game plugins wire their stores into it.
2. Event Plugin (event_plugin.cpp)
Defines components: "EventSwapper"
Requires components: none
init_fn:
- Allocates EventSwapper
- Registers and binds to the world
tick_fn:
- Calls event_swapper->swap_all() to swap all registered event queue buffers
- This must run before any other tick logic that polls events. The engine runs tick functions in system load order, and the dependency graph ensures this plugin is loaded before plugins that require "EventSwapper". So ordering is handled by the existing infrastructure.
frame_fn: nullptr
shutdown_fn:
- Deletes EventSwapper
Notes: Game plugins that define specific EventQueue<T> instances register them with the swapper via event_swapper->add(queue, swap_queue<T>). The event plugin owns the swapper; game plugins wire their queues into it.
3. Interpolation Plugin (interpolation_plugin.cpp)
Defines components: "FrameAdvancer"
Requires components: none
init_fn:
- Allocates FrameAdvancer
- Registers and binds to the world
tick_fn:
- Calls frame_advancer->advance_all() to snapshot all registered Interpolated<T> values
- Same ordering consideration as the event plugin — this should run before game tick logic that modifies interpolated values. The dependency graph handles this.
frame_fn: nullptr
shutdown_fn:
- Deletes FrameAdvancer
Notes: Game plugins that define Interpolated<T> instances register them with the advancer via frame_advancer->add(interpolated, advance_interpolated<T>).
4. Resource Plugin (resource_plugin.cpp)
Defines components: "MeshStore", "TextureStore"
Requires components: none
init_fn:
- Allocates ResourceStore<MeshData> and ResourceStore<TextureData>
- Registers and binds both to the world
tick_fn: nullptr
frame_fn: nullptr
shutdown_fn:
- Deletes both stores
Notes: These are the two resource types currently defined in cask_core (MeshData, TextureData). Game plugins that need mesh or texture resources require these components and get<ResourceStore<MeshData>>() from the world. If a game needs additional resource types, it defines its own ResourceStore<T> in its own plugin — the resource plugin only covers the core types.
Dependency Graph
event_plugin          (defines: EventSwapper)
interpolation_plugin  (defines: FrameAdvancer)
resource_plugin       (defines: MeshStore, TextureStore)
entity_plugin         (defines: EntityTable, EntityCompactor; requires: EventSwapper)
The engine's dependency graph resolves this: event_plugin loads before entity_plugin. Interpolation and resource plugins have no dependencies and can load in any order.
Testing Strategy
Each plugin needs a test spec that verifies:
1. Registration and binding — after init, the component can be retrieved from the world and is non-null
2. Lifecycle behavior — for plugins with tick/frame behavior (event plugin swaps buffers, interpolation plugin advances frames), step the engine and verify the behavior occurred
3. Shutdown cleanup — after shutdown, resources are freed (no leaks)
Test structure: Create a test harness that mimics what the engine does: create a World, create a WorldHandle, call the plugin's init_fn, tick_fn, frame_fn, shutdown_fn directly. This avoids needing to actually load .so files in tests.
File naming: entity_plugin_spec.cpp, event_plugin_spec.cpp, interpolation_plugin_spec.cpp, resource_plugin_spec.cpp
Conventions
- Load the C++ skill before writing code
- TDD: write failing test first, then implement
- Catch2 BDD style: SCENARIO / GIVEN / WHEN / THEN
- No comments, no single-character names
- No nested conditionals
- PascalCase types, snake_case functions/variables, trailing underscore for private members
- .hpp for headers, .cpp for implementation
- Organize by domain: plugins/entity/, plugins/event/, plugins/interpolation/, plugins/resource/, spec/entity/, spec/event/, etc.
- Each plugin source file should be self-contained — one .cpp file per plugin producing one .dylib
- Follow the PluginInfo struct pattern from cask/abi.h exactly: name, defines_components, requires_components, counts, init/tick/frame/shutdown function pointers
- set_target_properties(plugin_name PROPERTIES PREFIX "") on every plugin target
What Success Looks Like
After this work, a game plugin like teapot_visual.cpp should be able to:
1. Declare requires_components = {"EntityTable", "EntityCompactor", "EventSwapper", "MeshStore", "TextureStore"}
2. In its init_fn, get<EntityTable>(), get<EventSwapper>(), etc. from the world — they're already allocated and bound
3. Define only its game-specific components (Transform, SpawnEvent queue, DestroyEvent queue, etc.)
4. Wire its game-specific stores into the compactor and swapper
5. Focus entirely on game logic, not infrastructure setup
The 40 lines of boilerplate in teapot_visual.cpp (lines 322-361) should shrink to ~10 lines of get calls plus game-specific setup.
---
