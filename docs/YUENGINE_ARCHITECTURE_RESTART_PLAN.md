# YuEngine Architecture Restart Plan

Status: phase-0 architecture baseline
Owner: 八云紫, 总架构师
Started: 2026-06-10
Repository baseline: `7dee47f 干净引擎文件夹`
Clean target: `C:\Steam\steamapps\common\TouhouNewWorld\YuEngine`
Old backup reference: `C:\Steam\steamapps\common\TouhouNewWorld\YuEngine_BACKUP_20260610_022934`

## 1. Product Definition

YuEngine is a general, commercial-grade, self-developed game engine. Its target qualities are:

- Maintainable: subsystem ownership is explicit and local.
- Extensible: upper layers depend on stable lower-layer interfaces, not on proof scripts.
- Verifiable: every layer has deterministic tests or harnesses that validate engine behavior.
- Performant: memory, threading, resource lifetime, rendering submission, and diagnostics cost are architecture constraints from the beginning.
- Commercially viable: the runtime must be able to support more than the TouhouNewWorld validation workload.

YuEngine is not:

- A TouhouNewWorld patch project.
- A mesh/resource viewer.
- A blue-screen demo launcher.
- A report generator or evidence dashboard.
- A continuation of the old `FrameRuntime.cpp` monolith.
- A clone of UE5 or Unity source code.

TouhouNewWorld is the first validation workload. Original-game evidence is acceptance input, not the architecture driver.

## 2. Non-Negotiable Restart Rules

- First phase is architecture only. No gameplay code.
- Old `Project` and old `YuEngine` are counterexamples, reference material, and evidence pools only.
- No module enters implementation until owner, lifecycle, inputs, outputs, performance constraints, tests, and UE/Unity comparison are written.
- Upper-layer evidence must not shape lower-layer runtime APIs.
- Diagnostics and JSON reports must not become core engine API.
- Renderer progress requires submit, present, and capture capability.
- UI business logic waits for a UI framework boundary.
- Audio business state waits for audio backend and mixer boundaries.
- Contract count is not engine progress.
- GitHub history is the delivery record. Architecture decisions, plans, gates, and future implementation changes must be committed and reviewable.

## 3. Target Layer Diagram

```text
L0 Host and Hardware
  OS, process, window, timer, file handles, dynamic libraries, CPU/GPU feature discovery

L1 Platform Services
  memory, logging sink, crash boundary, threading primitives, file system, input devices, audio device, graphics device discovery

L2 Engine Kernel
  module lifecycle, service registry, object identity, serialization base, event bus, task scheduler, diagnostics channel

L3 Low-Level Runtime Interfaces
  RHI, audio backend/mixer, input action layer, virtual file system, package reader, allocator policy, async IO

L4 Core Asset and Script Framework
  resource manager, asset handles, dependency graph, hot/reload policy, script VM/native bridge, reflection/table data

L5 Runtime World Systems
  world/scene lifecycle, entity/component or actor model, UI framework, animation, physics, render scene, audio scene, gameplay service facades

L6 Game Adapter Layer
  TouhouNewWorld title, save/new-game, scene, actor, camera, tutorial, original-script service mapping

L7 Verification and Tools
  oracle capture, frame diff, import/export, packaging, editor tooling, profiling, regression dashboards
```

Dependency rule: a layer may depend downward only. It may emit diagnostics outward through the diagnostics channel, but diagnostics may not own runtime behavior.

## 4. Implementation Order

This section records the 2026-06 restart bootstrap baseline. Current post-L0/L1
execution is governed by `docs/YUENGINE_LONG_PLAN_TEAM_EXECUTION.md` section 3:
capability lanes may proceed only when their exact prerequisites are stable and
their non-goals are explicit. This list must not be read as a whole-module
serialization rule.

1. Architecture baseline and gates.
2. Platform and hardware abstraction.
3. RHI, audio, input, file, thread, memory.
4. Engine kernel: modules, lifecycle, services, object identity, tasks, diagnostics channel.
5. Resource, serialization, script/native bridge.
6. Runtime scene, UI, animation, audio, render, gameplay facades.
7. TouhouNewWorld adapter for title/save/new-game/scene/actor/camera/tutorial.
8. Frame capture and oracle alignment.
9. Toolchain, data import/export, packaging, editor.

No gameplay implementation is allowed before step 6 has passed its gates.

## 5. Module Ownership Table

| Module | Owner role | Owns | Does not own | UE5 reference | Unity reference | Entry gate |
| --- | --- | --- | --- | --- | --- | --- |
| Platform | Engine core | process, window, timers, file handles, dynamic library loading, OS errors | game flow, resources, rendering policy | `Runtime/ApplicationCore`, platform folders | player/editor platform abstraction | window/event loop harness passes on target OS |
| Memory | Performance architecture | allocator strategy, budgets, lifetime tags, leak checks | object ownership policy above kernel | `Runtime/Core` allocators | native memory/profiler concepts | allocation API and accounting tests exist |
| Thread/Task | Performance architecture | worker threads, job queues, synchronization, frame scheduling primitives | gameplay update order | `Runtime/Core`, task graph concepts | job system concepts | deterministic task scheduling tests exist |
| File/VFS | Engine core | mounts, pack/loose lookup, async read contract, path normalization | asset semantics, original-game decisions | `Runtime/PakFile`, `Runtime/Projects` | AssetDatabase/package concepts | pack and loose fixture tests pass |
| RHI | Rendering architecture | device, swapchain, command submission, resource handles, present, capture hook | material semantics, scene traversal | `Runtime/RHI`, `RHICore`, `D3D12RHI`, `VulkanRHI` | SRP low-level graphics boundary | headless/null and real-device submit/present/capture harnesses pass |
| RenderCore | Rendering architecture | frame passes, render queues, shader/material binding contract, render scene interface | gameplay visibility rules | `Runtime/RenderCore`, `Renderer` | SRP pipeline instance/asset model | renders fixture scene through RHI with captured output |
| Audio | Audio architecture | device backend, mixer, voices, streaming, decoding, lifetime, latency | BGM/SE business IDs | `Runtime/AudioMixer` and audio codecs | AudioSource/AudioMixer concepts | mixer and voice lifecycle tests pass |
| Input | Runtime architecture | device events, action mapping, focus, replayable input frames | title menu behavior | `Runtime/InputCore`, input device modules | Input System package | deterministic input replay test passes |
| Engine Kernel | 总架构师 | module lifecycle, service registry, object identity, event bus, diagnostics channel | JSON report schemas, game state | `Runtime/Core`, `CoreUObject` boundary ideas | GameObject/component lifecycle as reference, not copy | lifecycle tests prove startup/update/shutdown order |
| Resource | Runtime architecture | asset handles, dependency graph, cache, load/unload, upload scheduling | original-game resource meaning | `Runtime/AssetRegistry`, `PakFile` | assets/scenes/packages | dependency and lifetime fixture tests pass |
| Script | Runtime architecture | VM boundary, native bridge, call ABI, script object lifetime | hard-coded title flow | Blueprint/Verse/VM boundary concepts | MonoBehaviour/script components as reference | VM fixture calls native services through stable interfaces |
| Scene/World | Runtime architecture | world lifecycle, actors/entities/components, scene streaming, update phases | specific TouhouNewWorld stage logic | `Runtime/Engine` world/actor concepts | scenes plus GameObject/component model | scene fixture loads, updates, unloads deterministically |
| UI | Runtime architecture | widget tree, layout, focus, input routing, animation, text, draw batching | title/save menu scripts | Slate/SlateCore/UMG layering | UI Toolkit/uGUI concepts | widget fixture can layout, route input, render, capture |
| Physics | Runtime architecture | collision world, queries, integration boundary, deterministic step policy | gameplay rules | `Runtime/PhysicsCore` | physics components | fixed-step fixture passes |
| Diagnostics | Quality architecture | logging, tracing, profiling, counters, report adapters | runtime ownership, public core API shape | `Runtime/TraceLog`, profiling systems | profiler/logging tools | diagnostics can be disabled or bounded without changing runtime behavior |
| Tools | Tools architecture | import/export, packaging, editor, build integration | runtime frame ownership | `Programs`, editor/tool separation | editor asset pipeline | tools run outside runtime boundary |
| Game Adapter | Game team after gates | original title/save/scene/actor/camera/tutorial mapping | engine core interfaces | game module pattern | project scripts/scenes | all lower required gates pass and original evidence is cataloged |

## 6. UE5 And Unity Reference Notes

Reference study is about responsibilities and boundaries, not copying implementation.

Local UE5 source root:

- `C:\Steam\steamapps\common\TouhouNewWorld\ue\Engine\Source`

Current local UE source repository snapshot:

- `7deeb413d3dc1fc034f48d1aacc0861301829d32` (`5.8.0 release`)

Initial UE5 modules to inspect:

- Platform/core: `Runtime/Core`, `Runtime/ApplicationCore`.
- Object/model/lifecycle: `Runtime/CoreUObject`, `Runtime/Engine`.
- RHI/rendering: `Runtime/RHI`, `Runtime/RHICore`, `Runtime/RenderCore`, `Runtime/Renderer`, `Runtime/D3D12RHI`, `Runtime/VulkanRHI`.
- Resources/packages: `Runtime/PakFile`, `Runtime/AssetRegistry`, `Runtime/Projects`.
- UI: `Runtime/SlateCore`, `Runtime/Slate`, `Runtime/UMG`.
- Audio: `Runtime/AudioMixer`, codec modules.
- Input: `Runtime/InputCore`, input device modules.
- Diagnostics/testing: `Runtime/TraceLog`, `Runtime/AutomationTest`.

Unity official references captured for boundary comparison:

- GameObject/component model: `Unity manual reference`
- Scenes as content containers: `Unity manual reference`
- Scriptable Render Pipeline concepts: `Unity manual reference`
- Input System package boundary: `Unity manual reference`

YuEngine decisions from these references:

- Use an engine-owned world/object model, but do not copy Unity's GameObject API.
- Keep rendering as a pipeline over low-level graphics commands, not as scene code directly calling backend APIs.
- Separate package/resource discovery from resource lifetime and upload.
- Separate runtime and tools/editor from the beginning.
- Keep UI as a framework with tree/layout/focus/rendering lifecycle before any title-screen business logic.

## 7. Commercial Capability Checklist

Platform:

- Window creation and lifecycle.
- Timer and frame clock.
- OS event pump.
- File and dynamic-library boundary.
- Crash/error boundary.
- Headless mode for tests.

RHI and rendering:

- Null RHI for deterministic tests.
- At least one real graphics backend.
- Device and swapchain lifecycle.
- Command submission.
- Present.
- Capture hook.
- Resource create/update/destroy.
- Shader/material binding contract.
- Frame pass model.

Audio:

- Device backend.
- Mixer graph.
- Voice/channel lifecycle.
- Streaming and decoding path.
- Latency policy.
- Resource lifetime.
- Test sink.

Input:

- Device event collection.
- Action mapping.
- Focus and routing policy.
- Frame input snapshot.
- Replay fixtures.

File/resource:

- VFS mounts.
- Pack and loose resource lookup.
- Asset handle and dependency graph.
- Cache and invalidation.
- Sync and async load contracts.
- Upload scheduling for GPU/audio resources.

Script:

- VM or interpreter boundary.
- Native bridge ABI.
- Script object lifetime.
- Error boundary.
- Deterministic fixture execution.

Object/world/scene:

- Object identity and lifetime.
- Scene/world creation, update, unload.
- Actor/entity/component ownership model.
- Update order.
- Event routing.

UI:

- Widget tree.
- Ownership and lifecycle.
- Layout.
- Focus/navigation/input routing.
- Text path.
- Animation/tween/state transition policy.
- Draw batching and material binding.

Animation:

- Clip/resource model.
- State machine or graph boundary.
- Update integration.
- Event markers.

Physics:

- Fixed-step policy.
- Collision world.
- Queries.
- Runtime integration boundary.

Diagnostics:

- Logging with levels and categories.
- Trace/profiler counters.
- Bounded report adapters.
- Runtime can run without reports changing behavior.

Tools:

- Import/export separated from runtime.
- Packaging separated from runtime.
- Editor added after runtime gates, not before.

Performance:

- Memory budgets and allocator policy.
- Threading model.
- Async loading policy.
- Frame scheduling budget.
- Renderer submission budget.
- Diagnostics overhead budget.
- Hot path data layout review.

## 8. Salvage Map From Old YuEngine

Old code is not copied into the new architecture until the relevant module gate exists.

Can be salvaged as evidence or test fixtures:

- Project manifest facts.
- VFS pack/loose lookup knowledge.
- `.sqasm` parsing observations.
- Native service names and call evidence.
- PE, `.pdata`, import table, dispatch table, and binary fingerprint evidence.
- D3D9/backend observations.
- DDS, material sampler, font atlas, depth/material-program evidence.
- Existing CTest split and long-contract knowledge.
- Oracle runbooks and frame/title evidence documents.

Can be salvaged later as implementation only after redesign:

- Small, isolated parsers with clear input/output and tests.
- Resource lookup algorithms if they can be detached from diagnostics/report output.
- Binary analysis utilities if they remain tools/evidence, not runtime core.

Must not be salvaged directly:

- `FrameRuntime.cpp`.
- Report-shaped runtime APIs.
- Script service state that substitutes for UI/audio/gameplay systems.
- Backend probes embedded in runtime flow.
- Business state inside core/service layers.
- Large aggregation files whose owner cannot be stated in one sentence.

## 9. Quarantine Map

The following old-backup areas are quarantine-only until a specific architecture owner approves extraction:

| Old area | Quarantine reason | Allowed use |
| --- | --- | --- |
| `src/yuengine/runtime/FrameRuntime.*` | monolithic aggregation of script, scene, renderer, backend, binary probes, reports, and business flow | failure example, migration checklist |
| `src/yuengine/script/ScriptRuntime.*` | mixes bytecode interpretation, evidence strings, service state, and report counters | script evidence, future VM requirements |
| `src/yuengine/resource/ResourceDiagnostics.*` | report JSON and resource checking are mixed into resource behavior | fixture facts, not runtime API |
| backend status docs | evidence-first renderer progress can mislead architecture | requirements catalog for RHI/render gates |
| title/mission/runtime docs | original-game flow facts are valuable but game-specific | acceptance input after engine runtime exists |
| old CMake/apps/tests | may encode accidental architecture and long-running contract habits | reference for verification split only |

## 10. Migration Plan Away From `FrameRuntime.cpp`

There is no direct migration by editing `FrameRuntime.cpp`. The new project replaces it with owned modules.

1. Freeze old backup as read-only reference.
2. Build an evidence index that lists facts extracted from old docs/code with source path and owner category.
3. Define new runtime interfaces for platform, kernel, RHI, audio, input, resource, script, scene, UI, diagnostics.
4. For each old `FrameRuntime.cpp` responsibility, assign one of:
   - discard;
   - evidence fixture;
   - tool-only utility;
   - future runtime feature after gate.
5. Recreate tests at the new module boundary before importing any old logic.
6. Port only the minimum proven algorithm into the owning module.
7. Delete all report-shaped API assumptions during porting.
8. Add a gate test proving the new owner behaves without depending on old frame runtime state.
9. Only after lower-layer gates pass may TouhouNewWorld adapter code call the new runtime interfaces.

Target replacement owners:

- Script facts -> `Script` module fixtures and VM/native bridge requirements.
- Resource facts -> `File/VFS` and `Resource` module fixtures.
- Renderer facts -> `RHI` and `RenderCore` requirements.
- Audio service calls -> `Audio` requirements.
- UI service calls -> `UI` requirements.
- Title/save/new-game facts -> `Game Adapter` acceptance tests.
- Reports -> `Diagnostics` adapters or external tools.

## 11. Acceptance Gates Per Layer

Layer 0-1 gate, host/platform:

- Window or headless app can start, tick, and shut down deterministically.
- File, timer, logging sink, thread primitive, and error boundary fixtures pass.
- No game, resource, rendering, UI, or script dependency.

Layer 2 gate, engine kernel:

- Modules declare dependencies and lifecycle.
- Startup/update/shutdown order is deterministic.
- Services are registered and resolved without global mutable business state.
- Diagnostics channel exists but does not define runtime ownership.

Layer 3 gate, low-level runtime:

- Null and real backends can be selected through the same interface where applicable.
- RHI can create resource, submit command, present, and capture.
- Audio can mix voices into a test sink.
- Input can replay recorded frames.
- VFS can resolve pack and loose fixtures.

Layer 4 gate, resource/script:

- Resource handles own lifetime and dependencies.
- Asset load/unload is testable without game-specific scripts.
- Script/native bridge calls services through stable interfaces.
- Serialization format has versioning/error behavior.

Layer 5 gate, world systems:

- Scene/world can create, update, and unload deterministic fixtures.
- UI fixture can layout, route input, render, and capture.
- Render scene submits through RenderCore/RHI only.
- Audio scene plays through Audio only.
- Physics/animation update order is explicit.

Layer 6 gate, game adapter:

- Original evidence catalog exists for title/save/new-game/scene/actor/camera/tutorial.
- Adapter cannot bypass runtime systems.
- Every original-game behavior test states which engine subsystem it validates.

Layer 7 gate, verification/tools:

- Oracle capture can compare output without becoming runtime dependency.
- Toolchain runs outside runtime and produces versioned artifacts.
- Performance counters have bounded overhead and can be disabled.

## 12. Performance Requirements

Performance requirements are architectural constraints, not late optimization tasks.

- Memory: every subsystem declares owner, allocator strategy, lifetime, and budget class.
- Threading: task queues and async IO must have deterministic shutdown and bounded synchronization.
- File/resource: cache behavior and invalidation must be explicit; no hidden global resource maps.
- RHI/render: submission path must avoid per-frame unbounded allocations; capture path must be optional.
- Audio: mixer thread or callback model must avoid blocking IO and uncontrolled allocation.
- Input: per-frame snapshots must be compact and replayable.
- Script: native bridge must define cost model and avoid stringly dynamic dispatch in hot paths.
- Diagnostics: logs/reports/traces are bounded, category controlled, and removable from hot runtime paths.
- Tests: performance smoke checks start early for allocator counts, frame allocations, job queue behavior, and diagnostics overhead.

## 13. Testing Strategy

Tests validate engine modules, not report existence.

Required test classes:

- Unit tests for pure parsers and value transforms.
- Module lifecycle tests for startup/update/shutdown.
- Fixture tests for VFS, resource handles, script/native bridge, input replay, audio sink, and RHI null backend.
- Backend smoke tests for real graphics/audio devices where available.
- Golden fixture diffs for stable outputs.
- Oracle tests only after runtime systems own behavior.
- Performance smoke tests for allocation count, task scheduling, resource load, render submit, and diagnostics overhead.

Testing rule:

- A report can be an output of a test, but a report passing is not the behavior under test.
- Every test must name the owning module and public interface it validates.
- Long-running evidence suites are separated from fast architecture gates.

## 14. Rule For Reintroducing Game-Specific Logic

TouhouNewWorld logic may enter only when all of these are true:

- Required lower-layer gates have passed.
- The behavior is backed by original evidence from `bin` or `resource`, or by old-backup evidence explicitly classified as evidence.
- The behavior is implemented in the Game Adapter layer.
- The adapter calls stable engine runtime interfaces instead of creating shortcuts.
- The test states both the original-game fact and the engine subsystem being validated.
- 门禁审查 approves the implementation entry condition.

If a game-specific need exposes a missing engine capability, work stops at the adapter boundary and returns to the owning engine module gate.

## 15. Team Split

- 八云紫: owns architecture direction, phase plan, subsystem boundaries, ADR decisions, and final implementation order.
- 红美铃: owns implementation gate review. Blocks work that lacks owner/lifecycle/I/O/performance/test/reference mapping.
- 八云蓝: owns UE/Unity/Godot reference research and maps mature engine boundaries into YuEngine decisions.
- 射命丸文: owns TouhouNewWorld original evidence catalog from `bin`, `resource`, and old evidence artifacts.
- 博丽灵梦: owns performance architecture for memory, threading, resource loading, render submission, diagnostics overhead, and hot-path data layout.
- 雾雨魔理沙: owns code review once implementation begins.
- 琪露诺, 大妖精, 露米娅, 米斯蒂娅: senior implementation engineers. They enter after architecture and gate tasks define slices.

## 16. Phase Plan

This phase plan is a restart bootstrap map. Current implementation lane release
must use the long-plan capability dependency graph and each lane's named
prerequisites, non-goals, and boundary-freeze guards.

Phase 0, architecture restart:

- Approve this restart plan.
- Produce subsystem reference map.
- Produce evidence catalog boundaries.
- Produce implementation gate checklist.
- Produce performance budget skeleton.
- Decide initial language/build/runtime shape only after the above are complete.

Phase 1, platform/kernel skeleton:

- Create build system and minimal runtime host.
- Implement platform, memory, logging, file, thread/task, module lifecycle.
- Add fast deterministic tests.

Phase 2, low-level runtime:

- Implement VFS/resource skeleton.
- Implement input replay boundary.
- Implement null RHI and first real RHI path.
- Implement audio test backend and mixer skeleton.

Phase 3, core runtime:

- Implement object/world/resource/script bridge skeleton.
- Add scene/world fixtures.
- Add diagnostics/profiling with bounded runtime cost.

Phase 4, runtime systems:

- Implement render scene, UI framework, animation, audio scene, and physics integration.
- Prove fixtures through module-owned behavior.

Phase 5, TouhouNewWorld validation:

- Add title/save/new-game/scene/actor/camera/tutorial adapter.
- Use original evidence as acceptance input.
- Add frame capture/oracle alignment.

Phase 6, tools and commercialization:

- Add import/export, packaging, editor/tooling, profiling dashboards, and release workflows.

## 17. GitHub Process

Every durable architecture or implementation change follows:

1. Inspect status before editing.
2. Make a focused change.
3. Verify with the strongest available non-interactive command.
4. Review diff.
5. Commit with a message describing the architecture outcome.
6. Push to GitHub.
7. Report commit hash and residual risks in Slock.

Current phase verification is documentation review plus Git diff, because no code or tests exist yet.

## 18. Immediate Follow-Up Tasks

Architecture restart cannot move to implementation until these parallel tasks are complete:

- Reference map: UE5/Unity subsystem boundaries by module.
- Evidence catalog: original-game facts separated from engine design.
- Gate checklist: module-entry and code-entry rules.
- Performance skeleton: early budgets and hot-path constraints.
- Build/language recommendation: only after the first four tasks report back.

Until these are complete, no gameplay, UI business, audio business, renderer demo, or resource viewer work is authorized.
