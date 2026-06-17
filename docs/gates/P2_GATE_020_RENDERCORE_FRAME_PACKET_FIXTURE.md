# P2-GATE-020: RenderCore Frame Packet Fixture

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NOT_APPROVED`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-001, P2-GATE-008, P2-GATE-017, P2-GATE-018, P2-GATE-019
Related decisions: ADR-0011
Source baseline: `eb664bb`
Proposal commit: `2bd618a`

## Layer

L5 lower-engine RenderCore frame packet fixture over the landed RenderCore
submission batch fixture and public RHI value contracts.

This gate proposes a bounded frame packet fixture. The fixture can accept one
caller-owned prepared submission batch request, execute it through the landed
`RenderSubmissionBatchFixture`, and record deterministic frame-level packet
status and counters. It is not a render graph, frame graph, renderer scheduler,
scene renderer, Resource owner, visual report, UI, World, Script, or Game
Adapter gate.

```text
caller-owned public YuRHI handles and prepared RenderCore pass requests
-> landed material binding fixture value fill
-> landed RenderCore submission batch fixture
-> bounded RenderCore frame packet fixture
-> later render graph, renderer scheduling, scene traversal, UI, World
```

## Current Reality

P2-GATE-017 landed a bounded RenderCore fixture pass at `13ccdb3`.
P2-GATE-018 landed a bounded material binding fixture at `b5620a3`.
P2-GATE-019 landed a bounded RenderCore submission batch fixture at `f4c3f64`.
The lower-engine render path can now prove multiple synthetic pass requests in a
deterministic batch over public `YuRHI` values. The engine still has no bounded
frame packet envelope, render graph, frame graph, material graph, render scene,
Resource-owned render lifetime, visual report, UI, World, Script, or Game
Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `784`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `31`;
- `ctest --preset windows-fast-gate -N -L Material`: `17`;
- `ctest --preset windows-fast-gate -N -L RHI`: `118`;
- `ctest --preset windows-fast-gate -N -L Resource`: `45`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `27`;
- `ctest --preset windows-fast-gate -N -L Upload`: `27`;
- `ctest --preset windows-fast-gate -N -L Fast`: `784`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `64`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `188`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Material`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`.

## Owns

This gate owns the proposal for:

- a narrow RenderCore frame packet fixture surface that depends only on public
  `YuRHI` values and landed public `YuRenderCore` fixture values;
- value-only frame packet descriptors for fixed packet capacity, frame ids,
  caller-owned submission batch request/result storage, and packet result
  storage;
- bounded frame packet snapshots for accepted, completed, failed, duplicate,
  and capacity-rejected packets;
- validation for zero frame id where invalid, null submission batch executor,
  null batch request storage, invalid batch request propagation, duplicate
  frame id, exhausted frame packet capacity, failed submission batch execution,
  and no mutation on validation failure;
- deterministic tests under default `windows-fast-gate` with `RenderCore`,
  `Material`, `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels;
- proof that a prepared RenderCore submission batch can be executed inside one
  bounded frame packet envelope without Resource, Streaming, scene, World, UI,
  Script, or Game Adapter ownership.

## Does Not Own

This gate does not own:

- render graph, frame graph, render queue threading, command-list parallelism,
  pass sorting, draw merging, transient resource aliasing, render target
  lifetime policy, visibility culling, scene traversal, or renderer-wide
  scheduling policy;
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
- RenderCore submission batch fixture owns a bounded list of prepared pass
  requests and deterministic sequential submission evidence;
- this proposed frame packet fixture owns only a bounded frame-level envelope
  around one prepared submission batch request;
- later renderer systems own render graph policy, scheduling, culling,
  batching, scene traversal, and material libraries;
- scene and World later own object traversal, visibility, transforms, and
  gameplay meaning;
- Game Adapter later owns title-specific render decisions.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep frame packets, render graph
policy, scene traversal, and gameplay render policy separate.

## Dependencies

Allowed dependencies:

- existing public `YuRHI` command, resource handle, binding, draw, submit,
  present, capture, status, and snapshot value contracts;
- landed public `YuRenderCore` fixture pass, material binding fixture, and
  submission batch fixture value contracts;
- `Tests/RenderCore` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- Package, File, Streaming, Resource load-state mutation, package parser
  implementations, original package readers, asset decode/import code, shader
  compiler/source tooling, material graph, render graph, frame graph, scene,
  UI, World, Script, Game Adapter, reports, screenshots, visual proof tooling,
  manual proof, or original-game evidence;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RenderCore frame packet headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public frame packet fixture contracts may expose value-only descriptors such as:

- public `YuRenderCore` submission batch fixture request/result/status/snapshot
  values;
- public `YuRHI` target, pipeline, buffer, texture, sampler, command, draw,
  status, and snapshot values already present in landed RenderCore requests;
- fixed-capacity frame packet limits, frame ids, counters, and snapshots;
- caller-owned output storage pointers or spans where existing local patterns
  require them.

Public frame packet fixture contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, Package ids, Streaming request ids,
  Resource load-state handles, scene ids, UI ids, report handles, screenshot
  artifacts, visual-proof types, or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  material graph nodes, package archive internals, or original-game package
  layout details.

Public `YuRHI`, `YuResource`, `YuPackage`, `YuStreaming`, and `YuWorld` headers
must not gain frame packet dependencies for this gate.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts and bounded storage for a synthetic RenderCore frame
   packet fixture.
2. Accept one caller-owned prepared `RenderSubmissionBatchFixtureRequest` and
   execute it through a caller-owned `RenderSubmissionBatchFixture`; do not load
   Resources, parse packages, read files, decode assets, compile shaders, sort
   passes, or traverse scenes.
3. Record deterministic frame packet result values such as frame id, batch
   status, completed entry count, failed entry count, last pass id, last
   material id, and snapshot counters.
4. Keep public RenderCore headers free of D3D11, DXGI, Win32, Platform,
   Resource, Package, Streaming, material graph, render graph, scene, UI,
   World, Script, and Game Adapter types.
5. Add deterministic fast tests for success, null executor, invalid batch
   request propagation, duplicate frame id, capacity, submission batch failure,
   no mutation on validation failure, and snapshot counters.
6. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission is
   expected for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, real package files, original-game packages, screenshots, reports,
logs, sleeps, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- discovery counts for `RenderCore`, `Material`, `RHI`, `Resource`,
  `Streaming`, `Upload`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no RenderCore
  frame packet tests;
- public-header scan for native/backend/Resource/Package/Streaming/material
  graph/render graph/frame graph/scene/World/Game leakage;
- production dependency scan proving the frame packet fixture does not depend
  on Package, File, Streaming, Resource load-state mutation, shader compiler,
  material graph, render graph, frame graph, scene, UI, World, Script, Game
  Adapter, original-game parser, or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- landed RenderCore submission batch fixture assertions through existing public
  APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  frame packet state, submission batch state, RenderCore pass state, RHI output
  handles, or caller-owned output values.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of frame packet proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- render graph, frame graph, material graph, scene traversal, UI, World,
  Script, or Game Adapter behavior as evidence for this gate.

## Non Goals

- No render graph or frame graph.
- No renderer scheduling policy.
- No render queue threading or command-list parallelism.
- No material graph.
- No shader compiler or shader source tooling.
- No Resource load completion state machine.
- No Package/File/Streaming ownership.
- No image, audio, mesh, shader, or material decoder.
- No scene traversal or World rendering.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in a narrow RenderCore frame packet
   fixture over landed submission batch and public RHI values without Resource,
   Streaming, scene, or renderer policy ownership;
2. implementability review confirms the landed RenderCore fixture pass,
   material binding fixture, submission batch fixture, and existing public RHI
   contracts can support the first slice locally;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
