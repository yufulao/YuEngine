# P2-GATE-025: RenderCore Render Graph Skeleton

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Combined lower-engine review
Depends on: P2-GATE-020, P2-GATE-019, P2-GATE-018, P2-GATE-017, P2-GATE-008
Related decisions: ADR-0011
Source baseline: `b8f74f0`
Proposal commit: `39b16a7`
Landed commit: `43dc361`

## Layer

L5 lower-engine RenderCore render graph declaration skeleton over landed
RenderCore fixture pass, material binding fixture, submission batch fixture,
frame packet fixture, and public RHI value contracts.

This gate proposes the first bounded RenderCore render graph skeleton. The
first slice can accept caller-owned pass declaration values, validate a fixed
pass order and dependency list, build a deterministic graph snapshot, and emit a
prepared submission batch request for the landed frame packet path. It is not a
renderer scheduler, frame graph, scene renderer, Resource owner, material graph,
command-list parallelism, report, UI, World, Script, or Game Adapter gate.

```text
caller-owned public YuRHI handles and prepared RenderCore pass requests
-> landed RenderCore submission batch fixture
-> landed RenderCore frame packet fixture
-> bounded RenderCore render graph declaration skeleton
-> later scheduling, transient resources, scene traversal, UI, World
```

## Current Reality

P2-GATE-017 landed a bounded RenderCore fixture pass at `13ccdb3`.
P2-GATE-018 landed a bounded material binding fixture at `b5620a3`.
P2-GATE-019 landed a bounded RenderCore submission batch fixture at `f4c3f64`.
P2-GATE-020 landed a bounded RenderCore frame packet fixture at `b275168`.
The lower-engine render path can now execute prepared synthetic pass requests
inside a deterministic frame packet over public `YuRHI` values. The engine still
has no render graph declaration, frame graph, scheduler, transient resource
lifetime, material graph, scene traversal, Resource-owned render lifetime, UI,
World, Script, or Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `832`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `40`;
- `ctest --preset windows-fast-gate -N -L Material`: `26`;
- `ctest --preset windows-fast-gate -N -L RHI`: `127`;
- `ctest --preset windows-fast-gate -N -L Resource`: `84`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `36`;
- `ctest --preset windows-fast-gate -N -L Upload`: `43`;
- `ctest --preset windows-fast-gate -N -L Fast`: `832`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `83`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `236`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Material`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`.

## Owns

This gate owns the proposal for:

- value-only RenderCore render graph status, operation, pass declaration,
  dependency declaration, graph request, graph record, and snapshot contracts;
- fixed-capacity RenderCore graph storage for pass declarations and dependency
  edges supplied by the caller;
- validation for empty graph, zero graph id, duplicate graph id, duplicate pass
  id, missing dependency, self dependency, dependency cycle, pass capacity
  overflow, dependency capacity overflow, and invalid prepared pass request;
- deterministic graph snapshots for accepted, rejected, prepared, dependency
  rejected, cycle rejected, capacity rejected, and released graph outcomes;
- preparation of a caller-owned `RenderSubmissionBatchFixtureRequest` from the
  declared graph order after full validation succeeds;
- release behavior that clears only RenderCore-owned graph declaration records
  and counters;
- deterministic tests under default `windows-fast-gate` with `RenderCore`,
  `Material`, `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels;
- proof that failed validation does not mutate RenderCore graph state, frame
  packet state, submission batch state, fixture pass state, RHI state, Resource
  state, Streaming state, scene state, or upper-engine state.

## Does Not Own

This gate does not own:

- render graph scheduling policy, frame graph execution, render queue threading,
  command-list parallelism, pass sorting beyond caller-provided order, draw
  merging, transient resource aliasing, render target lifetime policy,
  visibility culling, scene traversal, or renderer-wide scheduling policy;
- material graph evaluation, shader source tooling, shader compiler, shader
  permutation management, editor material assets, or renderer-wide material
  libraries;
- Resource load-state mutation, ResourceRegistry ownership, Package parsing,
  File IO, Streaming ownership, upload queue ownership, cache lifetime policy,
  image decode, texture import, mesh decode, or asset database behavior;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  render objects, hardware-only proof, or public native/backend leakage;
- UI, World, Script, gameplay, reports, screenshots, logs, sleeps, manual
  visual proof, original-game evidence, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- RHI owns backend-neutral GPU resource primitives and command execution;
- RenderCore fixture pass owns one bounded synthetic pass over caller-provided
  RHI values;
- Material binding fixture owns value grouping and validation for synthetic
  material metadata used by RenderCore fixture requests;
- RenderCore submission batch and frame packet fixtures own deterministic
  prepared-pass execution evidence;
- this proposed RenderCore render graph skeleton owns only declaration,
  dependency validation, snapshot, and prepared batch emission;
- later renderer systems own scheduling, culling, transient resource policy,
  batching, scene traversal, and material libraries;
- scene and World later own object traversal, visibility, transforms, and
  gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep graph declaration, graph
scheduling, RHI execution, material policy, scene meaning, and gameplay meaning
separate.

## Dependencies

Allowed dependencies:

- landed public `YuRenderCore` fixture pass, material binding fixture,
  submission batch fixture, and frame packet fixture value contracts;
- public `YuRHI` handle and descriptor value contracts already accepted by the
  landed RenderCore fixtures;
- new RenderCore-owned graph declaration value contracts and fixed-capacity
  records;
- `Tests/RenderCore` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuResource`, `YuStreaming`, `YuPackage`, `YuFile`, World, Script, UI, Game
  Adapter, parser, decoder, report, screenshot, visual proof, or original-game
  evidence dependencies from RenderCore core;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RenderCore graph headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public RenderCore graph contracts may expose value-only descriptors such as:

- RenderCore render graph status and operation values;
- graph id, pass id, dependency id, pass count, dependency count, and graph
  record slot values;
- caller-owned prepared RenderCore pass request pointers or arrays already used
  by landed submission batch fixtures;
- bounded snapshots for graph counters and last operation;
- query functions that copy graph records into caller-owned output buffers.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, Resource handles, Streaming requests,
  Package entries, material graph nodes, scene ids, UI ids, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types;
- graph scheduler internals, command-list worker state, transient resource
  allocation state, shader compiler handles, scene traversal iterators, or
  original-game renderer layout details.

Public `YuRenderCore` graph headers must remain free of Resource, Streaming,
Package, File, D3D11, DXGI, Platform, material graph, scene, UI, World, Script,
and Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts for RenderCore graph status, operation, pass
   declaration, dependency declaration, request, record, and snapshot.
2. Add bounded RenderCore storage for graph declaration records and dependency
   edges.
3. Validate graph id, pass ids, dependency endpoints, duplicate declarations,
   missing dependencies, self dependencies, cycles, pass capacity, dependency
   capacity, and prepared pass request validity before mutation.
4. Prepare a caller-owned `RenderSubmissionBatchFixtureRequest` in declaration
   order only after full graph validation succeeds.
5. Add query and release behavior that returns or clears only RenderCore-owned
   graph declaration metadata.
6. Keep RenderCore public and production code free of Resource, Streaming,
   Package, File, material graph, render scheduler, scene, UI, World, Script,
   Game Adapter, native/backend, report, screenshot, log, sleep, and
   original-game evidence dependencies.
7. Add deterministic fast tests for graph declaration success, prepared batch
   emission, graph query, release, duplicate graph id, duplicate pass id,
   missing dependency, self dependency, cycle rejection, pass capacity overflow,
   dependency capacity overflow, invalid pass request, and no mutation on
   failed validation.
8. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission is
   expected for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, real package files, original-game packages, screenshots, reports,
logs, sleeps, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `RenderCore_Graph` or equivalent tests proving the new boundary;
- discovery counts for `RenderCore`, `Material`, `RHI`, `Resource`,
  `Streaming`, `Upload`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no RenderCore
  graph tests;
- public-header scan for native/backend/Resource/World/Game leakage;
- production dependency scan proving `YuRenderCore` does not gain Resource,
  Streaming, Package, File, material graph, scene, UI, World, Script, Game
  Adapter, original-game renderer, or backend-native dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic graph declaration and dependency validation assertions in fast
  tests;
- graph snapshot, query, release, and prepared batch emission assertions through
  public RenderCore value APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  graph state, frame packet state, submission batch state, fixture pass state,
  RHI state, Resource state, Streaming state, or upper-layer state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of RenderCore graph proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- Resource loading, package parsing, real image/audio/mesh decode, RHI backend
  execution, material graph, render scheduler, frame graph, scene traversal, UI,
  World, Script, or Game Adapter behavior as evidence for this gate.

## Non Goals

- No render scheduler or frame graph execution.
- No command-list parallelism or worker graph.
- No transient resource aliasing or render target lifetime policy.
- No Resource, Package, File, or Streaming ownership.
- No material graph or shader compiler/source tooling.
- No scene traversal, visibility culling, World, UI, Script, gameplay, report,
  screenshot, log, sleep, manual proof, or Game Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after a combined lower-engine review
confirms the proposal stays in RenderCore-owned value contracts and bounded graph
declaration records over existing fixture values, remains implementable with the
landed frame packet/submission batch path, and preserves deterministic
fast-gate proof without native/backend, Resource, scene, report, screenshot,
log, sleep, manual, original-game, or hardware-only evidence.
