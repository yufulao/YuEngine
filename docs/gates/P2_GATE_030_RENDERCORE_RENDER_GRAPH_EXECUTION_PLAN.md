# P2-GATE-030: RenderCore Render Graph Execution Plan

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Split lower-engine review
Depends on: P2-GATE-025, P2-GATE-020, P2-GATE-019, P2-GATE-017, P2-GATE-004, ADR-0011
Related decisions: ADR-0011
Source baseline: `8e61fd4`
Proposal commit: `221423f`
Approval evidence: ENG-151A, ENG-151B, ENG-151C, ENG-151D, ENG-151E, and ENG-151F proposal review PASS.

## Layer

L5 lower-engine RenderCore execution-plan boundary over the landed RenderCore
render graph skeleton, frame packet fixture, submission batch fixture, fixture
pass, and public RHI value contracts.

This gate proposes the first bounded RenderCore render graph execution-plan
slice. The first slice may accept a successful `RenderGraphSkeleton::Prepare`
result, validate caller-owned frame execution storage, record deterministic
execution-plan metadata, execute the prepared submission batch through the
landed frame packet fixture, expose query/release snapshots, and prove failure
paths do not mutate unrelated engine state. It is not a renderer scheduler,
frame graph system, command-list threading gate, transient resource policy,
scene renderer, material graph, report, UI, World, Script, or Game Adapter
gate.

```text
landed RenderGraphSkeleton prepared batch
-> RenderCore execution-plan value boundary
-> landed RenderFramePacketFixture execution path
-> deterministic plan metadata and counters
-> later scheduler, frame graph, transient resources, scene traversal, UI, World
```

## Current Reality

P2-GATE-017 landed a bounded RenderCore fixture pass at `13ccdb3`.
P2-GATE-019 landed a bounded submission batch fixture at `f4c3f64`.
P2-GATE-020 landed a bounded frame packet fixture at `b275168`.
P2-GATE-025 landed the render graph skeleton at `43dc361`. The skeleton can
validate caller-owned graph declarations and prepare a submission batch request
in declaration order, but it intentionally does not execute that prepared graph
as a graph-level unit.

P2-GATE-029 landed at `0a22dac` and the current queue was marked landed at
`8e61fd4`. The accepted post-landing evidence for the lower-engine baseline is:

- full `windows-fast-gate` PASS `901/901`;
- `AudioResource` `8`;
- `Audio` `53`;
- `Resource` `119`;
- `Streaming` `36`;
- `Upload` `43`;
- `RHI` `148`;
- `RenderCore` `54`;
- `Material` `40`;
- `Fast` `901`;
- `PerformanceSmoke` `108`;
- `EvidenceOracle` `305`;
- default `HardwareSmoke` `0`;
- `windows-hardware-smoke` `7` with no RenderCore graph execution entries.

The missing lower-engine boundary is a RenderCore-owned execution-plan record
for a prepared graph. Today the graph skeleton can prepare a batch, and the
frame packet fixture can execute a batch, but there is no reviewed RenderCore
unit that validates the prepared skeleton result, records execution-plan
metadata, calls the landed frame packet fixture once, and proves deterministic
query/release/counter behavior.

## Approval Evidence

Approved after ENG-151A, ENG-151B, ENG-151C, ENG-151D, ENG-151E, and ENG-151F
proposal reviews PASS.

Review evidence:

- proposal commit `221423f84f9c1753522824231c2527cb3f3c7ca6` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 221423f^..221423f` passed;
- review work was read-only and made no source, doc, commit, push, approval, or
  implementation changes;
- boundary and quality review confirmed the proposal stays in RenderCore-owned
  execution-plan metadata over landed `RenderGraphSkeleton` prepared batches
  plus `RenderFramePacketFixture` execution and found no
  `NEEDS_ARCHITECTURE` blocker;
- implementability review confirmed existing `RenderGraphSkeleton::Prepare`,
  `RenderFramePacketFixture::Execute`, `RenderSubmissionBatchFixture`,
  `RenderFixturePass`, and public RHI value contracts are sufficient for the
  first slice without new backend-native or RHI contract requirements;
- test and preset review confirmed deterministic focused
  `RenderCore_GraphExecutionPlan` evidence, `CMakePresets` no-drift, label
  discovery, and `windows-hardware-smoke` isolation are sufficient;
- dependency and public-leak review confirmed current RenderCore public and
  production surfaces remain free of Resource, Streaming, Package, File, World,
  UI, Script, Game Adapter, D3D, DXGI, COM, Platform, and native/backend leaks;
- candidate consistency review confirmed this gate does not duplicate
  P2-GATE-025 skeleton declaration/dependency validation and does not widen
  into scheduler, frame graph, transient resource, scene, or upper-engine
  systems;
- performance and hardware evidence review confirmed fixed-capacity records,
  capacity-before-mutation validation, caller-owned storage, deterministic fast
  evidence, `PerformanceSmoke`/`EvidenceOracle` proof, and hardware-smoke
  isolation are acceptable for the first slice;
- proposal discovery matched the reviewed baseline: default fast gate `901`,
  `RenderCore` `54`, `Material` `40`, `RHI` `148`, `Resource` `119`,
  `Streaming` `36`, `Upload` `43`, `Fast` `901`, `PerformanceSmoke` `108`,
  `EvidenceOracle` `305`, default `HardwareSmoke` `0`, and
  `windows-hardware-smoke` `7` with no RenderCore graph execution-plan entries.

## Owns

This gate owns the proposal for:

- RenderCore graph execution-plan status, operation, request, record, result,
  and snapshot value contracts;
- fixed-capacity RenderCore execution-plan records keyed by caller-provided
  plan id and graph id;
- validation for null frame executor, invalid frame id, invalid plan id, failed
  skeleton prepare result, missing prepared batch request, missing pass result
  storage, zero pass count, duplicate plan id, duplicate graph execution, and
  plan record capacity overflow;
- deterministic execution of exactly one prepared skeleton batch through the
  landed `RenderFramePacketFixture` path;
- propagation of frame packet fixture failure without converting it into
  success;
- query and release behavior for execution-plan metadata only;
- snapshots for accepted, rejected, frame-failed, duplicate, capacity, query,
  release, and reset outcomes;
- proof that rejected execution plans do not mutate graph skeleton records,
  frame packet records, submission batch records, fixture pass records, RHI
  state, Resource state, Streaming state, scene state, or upper-engine state;
- deterministic tests under default `windows-fast-gate` with `RenderCore`,
  `Material`, `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- render scheduler policy, frame graph system, command-list parallelism,
  worker graph, pass sorting beyond the prepared skeleton order, draw merging,
  transient resource aliasing, render target lifetime policy, visibility
  culling, scene traversal, renderer-wide scheduling, or presentation policy;
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
- RenderCore submission batch and frame packet fixtures own deterministic
  prepared-pass execution evidence;
- RenderCore render graph skeleton owns declaration validation and prepared
  batch emission;
- this proposed execution-plan boundary owns only graph-level validation,
  metadata, and a single deterministic bridge into the landed frame packet
  fixture;
- later renderer systems own scheduling, culling, transient resource policy,
  batching, scene traversal, material libraries, and presentation policy;
- scene and World later own object traversal, visibility, transforms, and
  gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep graph declaration, graph-level
execution-plan metadata, scheduling, RHI execution, material policy, scene
meaning, and gameplay meaning separate.

## Dependencies

Allowed dependencies:

- landed public `YuRenderCore` render graph skeleton, frame packet fixture,
  submission batch fixture, fixture pass, and material binding fixture value
  contracts;
- public `YuRHI` handle and descriptor value contracts already accepted by the
  landed RenderCore fixtures;
- new RenderCore-owned graph execution-plan value contracts and fixed-capacity
  records;
- `Tests/RenderCore` focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuResource`, `YuStreaming`, `YuPackage`, `YuFile`, World, Script, UI, Game
  Adapter, parser, decoder, report, screenshot, visual proof, or original-game
  evidence dependencies from RenderCore production graph execution-plan code;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RenderCore execution-plan headers;
- generated reports, screenshots, logs, sleep timing, manual visual
  inspection, hardware-only proof, or original-game output as evidence.

## Public Contract Boundary

Public RenderCore graph execution-plan contracts may expose value-only
descriptors such as:

- execution-plan status and operation values;
- plan id, graph id, frame id, pass count, record slot, and execution counters;
- caller-owned prepared skeleton result values;
- caller-owned frame packet result storage;
- bounded snapshots for plan counters and last operation;
- query functions that copy execution-plan records into caller-owned output
  buffers.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, Resource handles, Streaming requests,
  Package entries, material graph nodes, scene ids, UI ids, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types;
- graph scheduler internals, command-list worker state, transient resource
  allocation state, shader compiler handles, scene traversal iterators, or
  original-game renderer layout details.

Public `YuRenderCore` execution-plan headers must remain free of Resource,
Streaming, Package, File, D3D11, DXGI, Platform, material graph, scene, UI,
World, Script, and Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts for RenderCore graph execution-plan status,
   operation, request, record, result, and snapshot values.
2. Add bounded RenderCore storage for execution-plan records.
3. Validate plan id, graph id, frame id, prepared skeleton result, prepared
   batch request, pass/result storage, duplicate plan id, duplicate graph
   execution, and capacity before mutation.
4. Execute exactly one prepared skeleton batch through the landed
   `RenderFramePacketFixture` path after validation succeeds.
5. Return explicit status and frame packet status when frame execution fails.
6. Add query and release behavior that returns or clears only RenderCore-owned
   execution-plan metadata.
7. Keep public and production code free of Resource, Streaming, Package, File,
   material graph, render scheduler, scene, UI, World, Script, Game Adapter,
   native/backend, report, screenshot, log, sleep, and original-game evidence
   dependencies.
8. Add deterministic fast tests for success, failed skeleton result, null frame
   executor, invalid frame id, invalid prepared batch, duplicate plan id,
   duplicate graph execution, capacity overflow, frame failure propagation,
   query, release, snapshot counters, and no mutation on failed validation.
9. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission
   is expected for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, real package files, original-game packages, screenshots, reports,
logs, sleeps, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `RenderCore_GraphExecutionPlan` or equivalent tests proving the new
  boundary;
- discovery counts for `RenderCore`, `Material`, `RHI`, `Resource`,
  `Streaming`, `Upload`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no RenderCore
  graph execution-plan tests;
- public-header scan for native/backend/Resource/World/Game leakage;
- production dependency scan proving `YuRenderCore` does not gain Resource,
  Streaming, Package, File, material graph, scene, UI, World, Script, Game
  Adapter, original-game renderer, or backend-native dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic execution-plan validation assertions in fast tests;
- graph execution-plan record, query, release, snapshot, and frame packet
  propagation assertions through public RenderCore value APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  graph skeleton state, frame packet state, submission batch state, fixture pass
  state, RHI state, Resource state, Streaming state, or upper-layer state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of RenderCore graph execution-plan proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- Resource loading, package parsing, real image/audio/mesh decode, RHI backend
  execution, material graph, render scheduler, frame graph, scene traversal,
  UI, World, Script, or Game Adapter behavior as evidence for this gate.

## Non Goals

- No render scheduler or frame graph system.
- No command-list parallelism or worker graph.
- No transient resource aliasing or render target lifetime policy.
- No Resource, Package, File, or Streaming ownership.
- No material graph or shader compiler/source tooling.
- No scene traversal, visibility culling, World, UI, Script, gameplay, report,
  screenshot, log, sleep, manual proof, hardware-only proof, or Game Adapter
  behavior.

## Required Review Before Approval

Before `APPROVED_FOR_FIRST_SLICE`, reviewers should independently confirm:

- the proposed boundary is narrower than a renderer scheduler and only bridges a
  prepared skeleton result into the landed frame packet fixture;
- the first slice can be implemented with existing RenderCore and RHI public
  value contracts;
- no Resource, Streaming, Package, File, material graph, scene, UI, World,
  Script, Game Adapter, backend-native, report, screenshot, log, sleep, manual
  proof, hardware-only proof, or original-game evidence dependency is required;
- deterministic fast tests and label evidence are sufficient for the first
  slice;
- CMake presets do not need to change unless labels for new tests require root
  CMake target updates.
