# YuEngine Module Entry Gates

Status: Phase 0 gate rule set  
Owner: 红美铃, implementation gate review  
Source baseline: `attachments/YUENGINE_ARCHITECTURE_RESTART_PLAN.md`  
Applies to: all YuEngine architecture, gate, and implementation tasks after the 2026-06-10 restart

## 1. Gate Purpose

This document defines when a YuEngine module may enter implementation.

YuEngine is a commercial-grade general game engine. TouhouNewWorld is a validation workload and evidence source only. A module is not approved because it helps a demo run, reproduces a visible game screen, produces a report, or matches old `FrameRuntime.cpp` behavior. A module is approved only when its engine responsibility is bounded, testable, comparable to mature engine boundaries, and compatible with the layer dependency rules.

## 2. Gate Decision States

- `BLOCKED`: implementation must not start.
- `NEEDS_ARCHITECTURE`: architecture owner must clarify ownership, layer, lifecycle, dependency, or allowed first slice.
- `NEEDS_PERFORMANCE`: performance architecture must define budget, hot-path assumptions, or measurement approach.
- `NEEDS_EVIDENCE`: original-game or old-backup facts are required, but only as validation input.
- `APPROVED_FOR_FIRST_SLICE`: the module may implement only the approved first slice.
- `APPROVED_FOR_NEXT_SLICE`: a later slice may proceed after previous gate tests pass.

Only `APPROVED_FOR_FIRST_SLICE` or `APPROVED_FOR_NEXT_SLICE` permits implementation work.

## 3. Universal Module Entry Checklist

Every module gate proposal must answer all items below before implementation.

### 3.1 Charter

- Module name and target layer (`L0` through `L7` from the restart plan).
- Owner role and reviewer roles.
- What the module owns.
- What the module explicitly does not own.
- UE5 analogue and Unity analogue, with responsibility comparison rather than copied API shape.
- Why the module belongs at this layer.

### 3.2 Lifecycle

- Creation, startup, update/tick if any, shutdown, and destruction order.
- Dependency initialization order.
- Failure path and teardown behavior.
- Resource lifetime ownership.
- Thread or callback lifetime, if any.
- Deterministic shutdown requirement.

### 3.3 Inputs And Outputs

- Public input types and who produces them.
- Public output types and who consumes them.
- Ownership and mutability of data crossing the boundary.
- Sync/async behavior.
- Error behavior.
- Serialization/versioning behavior, where applicable.
- Diagnostics emitted, without diagnostics owning runtime behavior.

### 3.4 Dependency Rules

- A layer may depend downward only.
- Runtime behavior may emit diagnostics through the diagnostics channel, but diagnostics must not define or own behavior.
- Game Adapter code must not be referenced by lower engine layers.
- Original-game evidence must not shape lower-layer API design.
- Tools/editor code must not become runtime dependency.
- Backend probes must not be embedded in runtime flow.
- Old `FrameRuntime.cpp` responsibilities must be classified as discard, evidence fixture, tool utility, or future module feature before any porting.

### 3.5 Performance Constraints

Each module must state:

- Budget class: cold path, frame path, hot path, IO path, backend callback, or tool-only.
- Allocation policy and whether per-frame allocation is allowed.
- Threading model and synchronization boundary.
- Cache/lifetime policy, if resources are held.
- Diagnostics overhead policy and disable/bounding behavior.
- Minimum performance smoke test or measurement.

If these are not defined, the gate state is `NEEDS_PERFORMANCE`.

### 3.6 Tests

Each module must identify:

- Fast gate tests that run without original-game data.
- Fixture tests for public module behavior.
- Negative tests for invalid lifecycle, invalid input, or missing dependency.
- Performance smoke tests where the module can affect frame, IO, memory, threading, rendering, audio, or diagnostics cost.
- Any long evidence/oracle suite, separated from fast gates.

A report may be a test output. A report passing is not the behavior under test.

### 3.7 Allowed First Slice

Each module must define the smallest useful first slice that proves its boundary without using gameplay, fake UI, resource viewers, report-driven runtime, or old monolith code.

The first slice must:

- Exercise the public interface.
- Prove lifecycle and teardown.
- Have deterministic tests or harnesses.
- Avoid upper-layer shortcuts.
- Avoid TouhouNewWorld-specific behavior unless this is the Game Adapter layer and lower gates have passed.

## 4. Immediate Global Blocks

The following work is blocked until the named lower gates pass.

| Work request | Gate result | Unblocks only when |
| --- | --- | --- |
| Gameplay, title flow, save/new-game, actor/camera/tutorial logic | `BLOCKED` | Game Adapter gate opens after L0-L5 required gates pass and evidence catalog exists |
| Fake menu or title-screen UI business | `BLOCKED` | UI framework gate proves widget tree, layout, focus/input routing, render, and capture |
| Audio BGM/SE business state | `BLOCKED` | Audio gate proves backend, mixer, voice lifecycle, streaming/decoding, and test sink |
| Renderer demo, blue-screen demo, or mesh viewer | `BLOCKED` | RHI and RenderCore gates prove resource, submit, present, capture, and fixture render path |
| Resource viewer or original resource browser | `BLOCKED` | File/VFS and Resource gates prove pack/loose lookup, handles, dependency graph, lifetime, cache behavior |
| Diagnostics/report JSON as core API | `BLOCKED` | Diagnostics is bounded output channel/adapter only, not runtime owner |
| Old `FrameRuntime.cpp` migration | `BLOCKED` | Responsibilities are re-owned by new modules and tested at new boundaries |
| Contract count or report count as progress | `BLOCKED` | Progress must be module-owned behavior plus tests |
| Build/language/runtime shape decision | `NEEDS_ARCHITECTURE` | Reference map, evidence boundary, gate checklist, and performance skeleton report back |

## 5. Layer Gates

### L0-L1 Host, Hardware, Platform Services

Entry conditions:

- No dependency on game, resource, rendering policy, UI, script, or original-game evidence.
- Window or headless host lifecycle is defined.
- OS event pump, timer, file handle, dynamic library, logging sink, thread primitive, and error/crash boundary are in scope.
- Tests prove start, tick, and shutdown deterministically.

Allowed first slice:

- Headless app plus optional window harness with timer, logging sink, error boundary, and deterministic shutdown.

Forbidden:

- Rendering demo, resource loading semantics, game loop business, UI, script runtime.

### L2 Engine Kernel

Entry conditions:

- Module lifecycle and dependency declaration format are written.
- Service registry ownership and object identity rules are defined.
- Event bus/task scheduler/diagnostics channel boundaries are documented.
- No global mutable business state.
- Diagnostics channel exists as output; it does not own runtime behavior.

Allowed first slice:

- Kernel lifecycle test proving module startup/update/shutdown order and service registration/resolution.

Forbidden:

- JSON report schemas as core API, game state, resource semantics, backend probes.

### L3 Low-Level Runtime Interfaces

Entry conditions:

- Interface boundary is independent from scene/game behavior.
- Null or test backend is defined where possible.
- Real backend selection uses the same public interface where applicable.
- Tests cover lifecycle and error behavior.

Allowed first slice:

- Narrow backend harness: RHI null submit, audio test sink, input replay, VFS fixture, or async IO fixture depending on module.

Forbidden:

- Demo-first rendering, title menu input, BGM/SE business, game resource meaning.

### L4 Core Asset And Script Framework

Entry conditions:

- Asset handle lifetime, dependency graph, load/unload rules, script/native bridge ABI, and serialization/versioning are documented.
- Resource discovery is separated from resource lifetime and upload.
- Script calls services through stable interfaces.
- Tests do not require title flow or game adapter.

Allowed first slice:

- Resource handle fixture with dependency/lifetime test, or script VM fixture calling a stable native service.

Forbidden:

- Original-game script service state as runtime design, report-driven resource APIs, hard-coded title flow.

### L5 Runtime World Systems

Entry conditions:

- World/scene lifecycle and update phases are explicit.
- UI, animation, physics, render scene, audio scene, and gameplay facades each have their own boundary.
- Render scene submits through RenderCore/RHI only.
- Audio scene plays through Audio only.
- UI has tree/layout/focus/input/render lifecycle before business screens.

Allowed first slice:

- Deterministic scene/world fixture, UI fixture, animation/physics fixed-step fixture, render scene fixture, or audio scene fixture.

Forbidden:

- TouhouNewWorld stage logic, title/save menu, shortcuts around lower systems.

### L6 Game Adapter

Entry conditions:

- Required lower gates pass.
- Evidence catalog exists for title, save/new-game, scene, actor, camera, tutorial, and original-script service mapping.
- Each behavior states the original-game fact and the engine subsystem it validates.
- Adapter cannot bypass runtime systems.
- Implementation entry requires 门禁审查 approval.

Allowed first slice:

- Minimal adapter acceptance test for one cataloged behavior after lower systems own the behavior path.

Forbidden:

- Any lower-layer API change driven only by original-game convenience.

### L7 Verification And Tools

Entry conditions:

- Runtime can operate without tool/editor dependency.
- Oracle, frame diff, import/export, packaging, editor, profiling, and dashboards are external or adapter-level tools.
- Tool artifacts are versioned and bounded.

Allowed first slice:

- External verification harness or packaging/import fixture that consumes runtime outputs without owning runtime behavior.

Forbidden:

- Report dashboard as runtime core, editor-first engine design.

## 6. Module-Specific Entry Gates

| Module | Owner role | Required gate proof | Allowed first slice | Forbidden dependencies |
| --- | --- | --- | --- | --- |
| Platform | Engine core | process/window/headless lifecycle, timers, OS errors, file handles, dynamic library boundary | host harness starts, ticks, shuts down | resources, rendering policy, game flow, UI |
| Memory | Performance architecture | allocator strategy, lifetime tags, budgets, leak/accounting tests | allocation API plus accounting/leak fixture | object policy above kernel, hidden globals |
| Thread/Task | Performance architecture | worker/job model, sync boundary, deterministic shutdown | deterministic scheduling fixture | gameplay update order, blocking IO in hot path |
| File/VFS | Engine core | mounts, pack/loose lookup, async read contract, path normalization | pack/loose fixture tests | asset semantics, original-game decisions |
| RHI | Rendering architecture | device/swapchain, command submission, resource handles, present, capture hook | null RHI plus real-device submit/present/capture smoke where available | material semantics, scene traversal, blue-screen-only demo |
| RenderCore | Rendering architecture | frame passes, render queues, shader/material binding contract, render scene interface | fixture scene renders through RHI and capture | gameplay visibility rules, backend direct calls |
| Audio | Audio architecture | device backend, mixer, voices, streaming/decoding, lifetime, latency | mixer and voice lifecycle into test sink | BGM/SE business IDs, blocking IO callback |
| Input | Runtime architecture | device events, action mapping, focus, replayable input frames | deterministic input replay fixture | title menu behavior |
| Engine Kernel | 总架构师 | module lifecycle, services, object identity, event bus, diagnostics channel | lifecycle and service resolution fixture | JSON report schemas, game state, global mutable business |
| Resource | Runtime architecture | handles, dependency graph, cache, load/unload, upload scheduling | dependency and lifetime fixture | original-game resource meaning, diagnostics-owned behavior |
| Script | Runtime architecture | VM boundary, native bridge ABI, script object lifetime, error boundary | script fixture calls native service through stable interface | hard-coded title flow, stringly hot-path dispatch without cost model |
| Scene/World | Runtime architecture | world lifecycle, actor/entity/component ownership, scene streaming, update phases | scene fixture loads, updates, unloads deterministically | TouhouNewWorld stage logic |
| UI | Runtime architecture | widget tree, layout, focus, input routing, text, animation, draw batching | widget fixture layouts, routes input, renders, captures | title/save menu scripts before framework |
| Physics | Runtime architecture | fixed-step policy, collision world, queries, integration boundary | fixed-step query/collision fixture | gameplay rules |
| Diagnostics | Quality architecture | logging, tracing, profiling, counters, bounded report adapters | diagnostics can be disabled/bounded without behavior change | runtime ownership, public core API shape |
| Tools | Tools architecture | import/export, packaging, editor/build integration outside runtime | tool fixture runs outside runtime boundary | runtime frame ownership |
| Game Adapter | Game team after gates | lower gates pass, evidence catalog exists, adapter calls stable runtime interfaces | one cataloged validation behavior through runtime systems | engine core interface shortcuts |

## 7. Module Gate Proposal Template

Every implementation slice must submit this before code begins:

```markdown
# Module Gate Proposal: <module>

Layer:
Owner:
Reviewers:
Source baseline:

## Owns

## Does Not Own

## UE/Unity Analogue

## Lifecycle

## Inputs

## Outputs

## Dependencies

## Forbidden Dependencies

## Performance Constraints

## Tests

## Allowed First Slice

## Non-Goals

## Evidence Inputs, If Any

## Gate Decision Requested
```

## 8. Review Flow

1. Architecture owner confirms layer, ownership, and dependency direction.
2. Performance architecture confirms budget class, allocation/threading expectations, hot-path assumptions, and smoke measurement.
3. Evidence owner confirms whether any original-game facts are acceptance inputs, not architecture drivers.
4. Gate review issues one gate state.
5. Implementation engineer may claim work only after an approved first slice exists.
6. Code review starts after implementation begins and checks that code stays within the approved gate.

## 9. Gate Review Default Decision

Until a module proposal satisfies this document, the default decision is:

`BLOCKED: module entry conditions not established.`

This default exists to prevent the two known failure modes: visual output that is not restoration, and evidence/process success that is not an engine.
