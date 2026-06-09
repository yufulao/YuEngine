# YuEngine Performance Gates

Status: phase-0 performance gate skeleton
Owner: Performance architecture
Started: 2026-06-10
Applies to: YuEngine architecture restart before implementation

## 1. Purpose

Performance is an implementation entry condition, not a late optimization pass.

This document defines the performance gates that must be attached to YuEngine
module-entry and code-entry reviews before implementation begins. It is scoped to
architecture constraints and deterministic checks. It does not authorize gameplay,
viewer/demo work, report-driven runtime, or migration from old `FrameRuntime.*`.

The initial language, build system, runtime shape, and concrete hardware targets
are still undecided. Until those decisions exist, budgets are expressed as gate
requirements: each owner must declare numeric limits, fixture scope, measurement
method, and pass/fail behavior before code enters implementation.

## 2. Non-Negotiable Rules

- Every module must declare memory owner, allocator strategy, lifetime tags, and
  budget class before implementation.
- Every lower-layer gate must contain at least one deterministic performance
  signal or budget check.
- Hot runtime paths must not depend on JSON reports, evidence dashboards, file
  probes, stringly dynamic dispatch, or original-game business facts.
- Diagnostics must be category-controlled, bounded, optional, and removable from
  hot runtime paths without changing runtime behavior.
- Async work must have deterministic shutdown, bounded queueing, and explicit
  cancellation or drain policy.
- Resource upload, render submission, audio callbacks, and input snapshots must
  avoid unbounded per-frame allocation.
- Capture and oracle paths are optional observers. They do not own runtime
  behavior and do not define public runtime APIs.

## 3. Required Gate Card

Each module-entry proposal must include this performance card:

| Field | Required content |
| --- | --- |
| Hot paths | Functions or phases expected to run per frame, per input frame, per audio callback, per job, or per resource chunk. |
| Memory owner | Owning module, allocation lifetime, allocator policy, lifetime tags, and peak budget. |
| Thread model | Calling thread, worker participation, allowed blocking points, shutdown behavior, and synchronization budget. |
| Resource flow | Sync/async IO, cache policy, staging/upload lifetime, invalidation, and max in-flight work. |
| Data layout | Handle model, hot data representation, stable IDs, string use policy, and layout review owner. |
| Diagnostics | Categories, counters, trace/log cost, disabled behavior, queue bounds, and report adapter boundary. |
| Deterministic signal | Fixture or smoke test that measures allocation count, peak bytes, queue depth, frame allocations, submit cost, callback allocation, or diagnostics overhead. |
| Failure rule | The exact condition that blocks implementation or blocks promotion to the next phase. |

If a field cannot be answered, the module cannot enter implementation.

## 4. Global Budget Classes

Budgets must be explicit even before final numeric targets are known. Owners must
classify each allocation and runtime cost into one of these classes:

| Class | Meaning | Gate expectation |
| --- | --- | --- |
| Startup | Process, platform, backend, module, and service initialization. | May allocate, but must be counted, owned, and released or intentionally retained. |
| Load | File reads, package lookup, decode, dependency resolution, staging, and GPU/audio upload preparation. | Must be bounded by fixture size, cache policy, queue depth, and cancellation/drain rules. |
| Frame | Main tick, world update, input snapshot, render scene build, render submit, and diagnostics emission. | No unbounded allocation; allocation count and peak transient bytes must be measured. |
| Callback | Audio callback or other external real-time callback. | No blocking IO; no uncontrolled allocation; synchronization must be bounded and documented. |
| Tool | Import/export, oracle capture, reports, profiling dashboards, and editor work. | Must stay outside runtime ownership unless a bounded runtime observer is explicitly approved. |

Each implementation slice must state which budget classes it touches.

## 5. Layer Gates

### L0-L1 Host And Platform Services

Required performance signals:

- Headless start/tick/shutdown fixture records startup allocation count, peak
  bytes, retained bytes, and shutdown leaks.
- Timer/frame-clock fixture records monotonic tick cost and jitter bounds for the
  selected platform harness.
- File primitive fixture records sync read cost for fixed-size files and verifies
  path normalization does not allocate in repeated hot lookups.
- Thread primitive fixture proves deterministic join/shutdown without orphaned
  workers or blocked process exit.
- Logging sink fixture proves disabled logging has bounded cost and no report
  dependency.

Blocking conditions:

- Global mutable resource maps appear in platform services.
- Platform APIs depend on rendering, UI, script, resource semantics, or
  TouhouNewWorld evidence.
- Logging/report output is required for runtime behavior.

### L2 Engine Kernel

Required performance signals:

- Module lifecycle fixture records allocation count and startup/update/shutdown
  order for a fixed module graph.
- Service registry fixture verifies lookup behavior and declares whether hot-path
  access uses stable handles, cached pointers, or another bounded mechanism.
- Task scheduler fixture records job count, queue depth, wake count, deterministic
  drain behavior, and shutdown latency under a fixed workload.
- Diagnostics channel fixture compares enabled and disabled counter/log paths.

Blocking conditions:

- Service lookup in a per-frame hot path is string-driven without a cached or
  stable lookup boundary.
- Task queues can grow without a declared bound or backpressure policy.
- Diagnostics channel owns runtime state or requires JSON/report schemas in core
  APIs.

### L3 Low-Level Runtime Interfaces

Required performance signals:

- VFS fixture resolves loose and packed paths with declared lookup allocation
  count, cache behavior, and max in-flight async reads.
- RHI null-backend fixture records resource create/update/destroy counts, command
  submit count, present call count, capture-hook behavior, and frame allocation
  count.
- First real-device RHI smoke test records submit/present/capture availability
  and separates backend cost from RenderCore policy.
- Audio test-sink fixture records mixer allocation count, buffer size, callback
  safety, and voice lifecycle cost.
- Input replay fixture records per-frame snapshot size, event count, replay cost,
  and allocation count.

Blocking conditions:

- Resource upload is hidden inside file lookup or game-specific code.
- RHI submission is proven only by a visual demo without submit/present/capture
  measurements.
- Audio callback performs blocking IO or uncontrolled allocation.
- Input events cannot be replayed deterministically from compact snapshots.

### L4 Core Asset And Script Framework

Required performance signals:

- Resource handle fixture records dependency graph size, load/unload allocation
  counts, retained cache bytes, invalidation behavior, and upload scheduling
  budget.
- Serialization fixture records version/error behavior and validates bounded
  parsing for fixed fixtures.
- Script/native bridge fixture records call cost model, argument marshaling
  allocation count, stable service lookup behavior, and error boundary cost.

Blocking conditions:

- Asset semantics are encoded into VFS path lookup.
- Script bridge dispatch is stringly dynamic in hot paths without stable IDs or
  pre-bound call sites.
- Resource lifetime is controlled by report output, diagnostics counters, or
  original-game business state.

### L5 Runtime World Systems

Required performance signals:

- World fixture records object/entity count, update phase order, per-frame
  allocations, and shutdown cleanup.
- Render scene fixture records visible item count, pass count, material/shader
  binding count, draw/submit batch count, and transient allocation budget.
- UI fixture records widget count, layout passes, input-routing cost, draw batch
  count, text-path allocation, and capture output.
- Audio scene fixture records voice count, mixer routing, streaming buffer budget,
  and frame-to-callback handoff cost.
- Physics/animation fixtures record fixed-step count, query cost, event marker
  cost, and update integration order.

Blocking conditions:

- Gameplay visibility, menu flow, BGM/SE business IDs, or original script state
  leak into core world, UI, render, or audio APIs.
- Render scene bypasses RenderCore/RHI for submission.
- UI framework is replaced by title-screen business logic.
- Audio scene bypasses audio backend and mixer ownership.

### L6 Game Adapter

Required performance signals:

- Adapter fixture states the original-game fact, engine subsystem being
  validated, expected budget class, and lower-layer gate it depends on.
- Adapter calls stable runtime interfaces and records adapter-only allocation and
  dispatch cost.

Blocking conditions:

- Adapter creates shortcuts around runtime systems.
- Missing engine capability is solved inside the adapter instead of returning to
  the owning module gate.
- Original-game evidence changes lower-layer API shape.

### L7 Verification And Tools

Required performance signals:

- Oracle/capture fixture proves capture can run as an observer without becoming a
  runtime dependency.
- Tool pipeline records import/export/package cost outside runtime ownership.
- Profiling/report fixtures prove runtime can run with diagnostics disabled and
  behavior unchanged.

Blocking conditions:

- Reports, dashboards, or oracle output are required for runtime behavior.
- Tooling owns runtime frame lifetime.
- Long-running evidence suites replace fast deterministic architecture gates.

## 6. Module-Specific Gate Additions

### Memory

- Define allocator API, ownership tags, budget classes, alignment policy, leak
  check behavior, and test accounting.
- Provide allocation accounting tests before any subsystem consumes the allocator.
- Track startup, load, frame, callback, and tool allocations separately.

Implementation is blocked if any subsystem allocates through an untracked global
path after the allocator policy exists.

### Thread And Task

- Define worker ownership, queue topology, job dependencies, wake strategy,
  synchronization primitives, max queue depth, shutdown policy, and test hooks.
- Provide deterministic scheduling tests before gameplay or resource pipelines use
  jobs.
- Prove cancellation or draining behavior for async IO and upload work.

Implementation is blocked if worker lifetime is owned by gameplay, reports, or an
unbounded background loop.

### File, VFS, Resource Load, And Upload

- Define mount lookup cost, path normalization policy, loose/pack priority, async
  read contract, cache invalidation, staging buffers, upload queues, and max
  in-flight requests.
- Separate package discovery from resource lifetime and GPU/audio upload.
- Provide fixtures for loose lookup, pack lookup, dependency load/unload, cache
  retention, and upload scheduling.

Implementation is blocked if resource meaning is embedded in file lookup or if
upload work is hidden inside diagnostics/report generation.

### RHI And Render Submission

- Define device/swapchain lifecycle, command ownership, resource handles,
  submission queue, frame pass contract, present behavior, capture hook, and null
  backend behavior.
- Record per-frame allocations, command submissions, resource transitions or
  equivalent backend operations, draw/batch counts, and capture overhead.
- Capture must be optional and must not alter render behavior when disabled.

Implementation is blocked if renderer progress is represented by a blue-screen
demo, viewer, or report without submit/present/capture harness evidence.

### Audio

- Define backend ownership, mixer graph, voice lifecycle, streaming buffers,
  decode boundaries, callback/thread model, latency policy, and test sink.
- Audio callback or mixer real-time path must avoid blocking IO and uncontrolled
  allocation.
- Provide lifecycle and sink fixtures before BGM/SE business mapping.

Implementation is blocked if title/menu audio business state enters the backend
or mixer boundary.

### Input

- Define device event collection, focus policy, action mapping, frame snapshot
  format, replay fixture, and per-frame size budget.
- Per-frame snapshots must be compact, deterministic, and independent of title
  menu behavior.

Implementation is blocked if input logic depends on game menu flow before the UI
and game adapter gates pass.

### Diagnostics

- Define log categories, counter registry, trace buffering, profiler markers,
  report adapters, disabled behavior, queue bounds, and sampling/drop policy.
- Provide enabled/disabled overhead fixtures and prove reports can be absent
  without changing runtime behavior.
- Diagnostics may observe allocations, jobs, resource loads, render submission,
  audio, and input. It does not own them.

Implementation is blocked if runtime APIs are shaped around JSON reports,
dashboards, oracle output, or old contract counts.

### Hot-Path Data Layout

- Identify hot data before implementation: per-frame update records, input
  snapshots, render submit items, resource handles, audio voices, task records,
  and diagnostics counters.
- Declare handle width, stable ID strategy, ownership, storage layout, iteration
  order, cache behavior, and string use policy.
- Use stable IDs or pre-bound call sites in hot paths; reserve strings and rich
  names for setup, tools, logs, or reports.
- Add layout assertions or fixture measurements once the language/runtime is
  chosen.

Implementation is blocked if a hot path uses unbounded maps, repeated string
lookup, report-shaped records, or per-element heap allocation without an approved
budget.

## 7. Review Outcomes

Performance architecture can approve implementation entry only when:

- The module performance card is complete.
- At least one deterministic signal is defined for the relevant layer gate.
- Budget classes and numeric limits are declared for the proposed fixture.
- Diagnostics disabled behavior is specified for hot paths.
- Runtime ownership is separated from tools, reports, oracle capture, and
  TouhouNewWorld evidence.

Performance architecture must block implementation when:

- A proposed slice cannot state its hot paths or lifecycle.
- Cost is described only as "small", "temporary", or "debug only" without a
  measurement fixture.
- Runtime behavior depends on diagnostics/report output.
- A lower layer takes dependency on game adapter facts.
- Old `FrameRuntime.*` shape is imported before the owning module gate exists.

## 8. Immediate Follow-Up

Before Phase 1 implementation starts, attach this performance gate document to
the module-entry checklist owned by implementation gate review. The first Phase 1
implementation slices should include fast deterministic smoke tests for:

- Platform start/tick/shutdown allocation accounting.
- Memory allocation accounting and leak behavior.
- Task scheduler queue depth and deterministic shutdown.
- File/VFS loose and pack lookup cost.
- Diagnostics enabled/disabled overhead.
