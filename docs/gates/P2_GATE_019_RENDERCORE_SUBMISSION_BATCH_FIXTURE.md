# P2-GATE-019: RenderCore Submission Batch Fixture

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-014, P2-GATE-017, P2-GATE-018
Related decisions: ADR-0011
Source baseline: `a78f671`
Proposal commit: `3c1fc89`

## Layer

L5 lower-engine RenderCore submission batch fixture over landed RenderCore pass,
material binding fixture, and public RHI value contracts.

This gate proposes a bounded submission batch fixture. The fixture can accept a
caller-owned, fixed-capacity list of prepared `RenderFixturePassRequest` values,
optionally populated by the landed material binding fixture, then execute them
through the landed `RenderFixturePass` in a deterministic order. It is not a
render graph, frame graph, renderer scene queue, material graph, Resource owner,
visual report, UI, World, Script, or Game Adapter gate.

```text
caller-owned public YuRHI handles and RenderCore pass requests
-> optional landed material binding fixture value fill
-> bounded RenderCore submission batch fixture
-> sequential landed RenderCore fixture pass execution
-> later render graph, scene traversal, renderer scheduling, UI, World
```

## Current Reality

P2-GATE-017 landed a bounded RenderCore fixture pass at `13ccdb3`.
P2-GATE-018 landed a bounded material binding fixture at `b5620a3`.
The lower-engine render path can now prove one synthetic pass and one material
binding over public `YuRHI` values. The engine still has no bounded multi-pass
submission fixture, render graph, frame graph, material graph, render scene,
Resource-owned render lifetime, visual report, UI, World, Script, or Game
Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `776`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `23`;
- `ctest --preset windows-fast-gate -N -L Material`: `9`;
- `ctest --preset windows-fast-gate -N -L RHI`: `110`;
- `ctest --preset windows-fast-gate -N -L Resource`: `45`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `27`;
- `ctest --preset windows-fast-gate -N -L Upload`: `27`;
- `ctest --preset windows-fast-gate -N -L Fast`: `776`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `61`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `180`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Material`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`.

## Owns

This gate owns the proposal for:

- a narrow RenderCore submission batch fixture surface that depends only on
  public `YuRHI` values and landed public `YuRenderCore` fixture values;
- value-only batch descriptors for fixed submission capacity, entry count,
  caller-owned pass request storage, pass ids, material ids, and result storage;
- bounded submission records and batch snapshots for accepted, completed,
  failed, and capacity-rejected entries;
- validation for zero entries, null pass executor, null request storage, invalid
  pass request propagation, duplicate pass id, exhausted batch capacity, failed
  RenderCore pass execution, and no mutation on validation failure;
- deterministic tests under default `windows-fast-gate` with `RenderCore`,
  `Material`, `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels;
- proof that multiple material-populated RenderCore fixture pass requests can be
  submitted in a bounded, deterministic sequence without Resource, Streaming,
  scene, World, UI, Script, or Game Adapter ownership.

## Does Not Own

This gate does not own:

- render graph, frame graph, render queue threading, command-list parallelism,
  pass sorting, draw merging, transient resource aliasing, render target
  lifetime policy, visibility culling, scene traversal, or renderer-wide
  scheduling policy;
- material graph evaluation, parameter reflection, shader source tooling,
  shader compiler, shader permutation management, editor material assets, or
  renderer-wide material libraries;
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
- this proposed submission batch fixture owns only a bounded list of prepared
  pass requests and deterministic sequential submission evidence;
- Resource, Package, File, and Streaming own asset identity, package/file
  staging, and upload preparation outside the batch fixture;
- later renderer systems own render graph policy, scheduling, culling, batching,
  scene traversal, and material libraries;
- scene and World later own object traversal, visibility, transforms, and
  gameplay meaning;
- Game Adapter later owns title-specific render decisions.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep RHI resources, material
binding, submission batching, scene traversal, and gameplay render policy
separate.

## Dependencies

Allowed dependencies:

- existing public `YuRHI` command, resource handle, binding, draw, submit,
  present, capture, status, and snapshot value contracts;
- landed public `YuRenderCore` fixture pass request, result, status, snapshot,
  and material binding fixture value contracts;
- `Tests/RenderCore` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- Package, File, Streaming, Resource load-state mutation, package parser
  implementations, original package readers, asset decode/import code, shader
  compiler/source tooling, material graph, render graph, scene, UI, World,
  Script, Game Adapter, reports, screenshots, visual proof tooling, manual
  proof, or original-game evidence;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RenderCore submission batch headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public submission batch fixture contracts may expose value-only descriptors such
as:

- public `YuRenderCore` fixture pass request/result/status/snapshot values;
- public `YuRHI` target, pipeline, buffer, texture, sampler, command, draw,
  status, and snapshot values already present in landed RenderCore requests;
- fixed-capacity submission limits, pass ids, material ids, per-entry status
  values, counters, and snapshots;
- caller-owned output storage pointers or spans where existing local patterns
  require them.

Public submission batch fixture contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, Package ids, Streaming request ids,
  Resource load-state handles, scene ids, UI ids, report handles, screenshot
  artifacts, visual-proof types, or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  material graph nodes, package archive internals, or original-game package
  layout details.

Public `YuRHI`, `YuResource`, `YuPackage`, `YuStreaming`, and `YuWorld` headers
must not gain submission batch dependencies for this gate.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts and bounded storage for a synthetic RenderCore
   submission batch fixture.
2. Accept a caller-owned list of already prepared `RenderFixturePassRequest`
   values; tests may use the landed material binding fixture to populate those
   values, but the batch fixture must not own material graph behavior.
3. Execute entries sequentially through the landed `RenderFixturePass` using
   existing public `YuRHI` values; do not load Resources, parse packages, read
   files, decode assets, compile shaders, sort passes, or traverse scenes.
4. Keep public RenderCore headers free of D3D11, DXGI, Win32, Platform,
   Resource, Package, Streaming, material graph, render graph, scene, UI,
   World, Script, and Game Adapter types.
5. Add deterministic fast tests for multi-entry success, zero entries, invalid
   input, duplicate pass id, batch capacity, RenderCore pass failure
   propagation, no mutation on validation failure, and snapshot counters.
6. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission is
   expected for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, real package files, original-game packages, screenshots, reports, logs,
sleeps, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- discovery counts for `RenderCore`, `Material`, `RHI`, `Resource`,
  `Streaming`, `Upload`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no RenderCore
  submission batch tests;
- public-header scan for native/backend/Resource/Package/Streaming/material
  graph/render graph/scene/World/Game leakage;
- production dependency scan proving the submission batch fixture does not
  depend on Package, File, Streaming, Resource load-state mutation, shader
  compiler, material graph, render graph, scene, UI, World, Script, Game
  Adapter, original-game parser, or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- RHI Null backend and landed RenderCore fixture pass assertions through
  existing public APIs;
- material binding fixture use only as value preparation for caller-owned pass
  requests;
- explicit capacity and validation counters proving failed paths do not mutate
  batch state, RenderCore pass state, RHI output handles, or caller-owned output
  values.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of submission batch proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- render graph, material graph, scene traversal, UI, World, Script, or Game
  Adapter behavior as evidence for this gate.

## Non Goals

- No render graph or frame graph.
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

1. boundary review confirms the scope stays in a narrow RenderCore submission
   batch fixture over public RHI, landed RenderCore pass, and material binding
   fixture values without Resource/Streaming/scene ownership;
2. implementability review confirms the landed RenderCore fixture pass,
   material binding fixture, and existing public RHI contracts can support the
   first slice locally;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
