# P2-GATE-018: Material Binding Fixture

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-014, P2-GATE-016, P2-GATE-017
Related decisions: ADR-0011
Source baseline: `616106d`
Proposal commit: `d918813`

## Layer

L5 lower-engine material binding fixture over landed RenderCore and public RHI
value contracts.

This gate proposes a bounded material binding fixture. The fixture can group a
caller-owned RHI pipeline handle, sampled texture binding, sampler binding, and
small fixed material constants into an explicit value contract, then feed a
landed `RenderFixturePass` request without owning Resources, compiling shaders,
loading assets, traversing scenes, producing screenshots, or defining game
render policy.

```text
caller-owned public YuRHI handles and bindings
-> bounded material fixture binding descriptor
-> landed RenderCore fixture pass request values
-> later render scene, material graph, renderer scheduling, UI, World
```

## Current Reality

P2-GATE-017 landed the first bounded RenderCore fixture pass at `13ccdb3`.
P2-GATE-014 supplies texture/sampler capture proof. P2-GATE-016 supplies
Resource upload queue proof into public RHI buffer/texture value APIs. The
engine still has no material binding boundary, material parameter fixture,
shader compiler, material graph, scene traversal, renderer-owned Resource
lifetime, visual report, UI, World, or Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `767`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `14`;
- `ctest --preset windows-fast-gate -N -L Material`: `0`;
- `ctest --preset windows-fast-gate -N -L RHI`: `101`;
- `ctest --preset windows-fast-gate -N -L Resource`: `45`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `27`;
- `ctest --preset windows-fast-gate -N -L Upload`: `27`;
- `ctest --preset windows-fast-gate -N -L Fast`: `767`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `59`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `171`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Material`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`.

## Owns

This gate owns the proposal for:

- a narrow material binding fixture surface under RenderCore or an equivalent
  lower-engine render module;
- value-only material fixture descriptors for caller-owned RHI pipeline,
  sampled texture, sampler, fixed constant bytes, material id, and slot bounds;
- bounded storage for material fixture records and material binding snapshots;
- validation for invalid pipeline, invalid sampled texture binding, invalid
  sampler binding, oversized constant payload, duplicate material id, exhausted
  fixture capacity, and failed RenderCore pass propagation;
- deterministic tests under default `windows-fast-gate` with `Material`,
  `RenderCore`, `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels;
- proof that material fixture binding can populate a RenderCore fixture pass
  request without Resource, Streaming, shader compiler, material graph, scene,
  World, UI, or Game Adapter ownership.

## Does Not Own

This gate does not own:

- material graph evaluation, parameter reflection, shader source tooling,
  shader compiler, shader permutation management, editor material assets, or
  renderer-wide material libraries;
- Resource load-state mutation, ResourceRegistry ownership, package parsing,
  file IO, Streaming ownership, upload queue ownership, cache lifetime policy,
  image decode, texture import, mesh decode, or asset database behavior;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  render objects, hardware-only proof, or public native/backend leakage;
- scene traversal, render scene ownership, visibility culling, batching,
  animation, UI, World, Script, gameplay, reports, screenshots, logs, sleeps,
  manual visual proof, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- RHI owns backend-neutral GPU resource primitives and command recording;
- RenderCore fixture pass owns bounded pass scheduling over caller-provided RHI
  values;
- this proposed material binding fixture owns only value grouping and
  validation for a synthetic material binding used by the RenderCore fixture;
- Resource, Package, File, and Streaming own asset identity, package/file
  staging, and upload preparation outside the material fixture;
- later renderer/material systems own shader graph policy, material libraries,
  reflection, and renderer scheduling;
- scene and World later own object traversal, visibility, transforms, and
  gameplay meaning;
- Game Adapter later owns title-specific render decisions.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep RHI resources, material
binding, render scene traversal, and gameplay render policy separate.

## Dependencies

Allowed dependencies:

- existing public `YuRHI` pipeline, texture, sampler, command, draw, status,
  and snapshot value contracts;
- landed public `YuRenderCore` fixture pass request, result, status, and
  snapshot value contracts;
- `Tests/RenderCore` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- Package, File, Streaming, Resource load-state mutation, package parser
  implementations, original package readers, asset decode/import code, shader
  compiler/source tooling, material graph, render scene, UI, World, Script,
  Game Adapter, reports, screenshots, visual proof tooling, manual proof, or
  original-game evidence;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RenderCore or material fixture headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public material fixture contracts may expose value-only descriptors such as:

- public `YuRHI` pipeline, sampled texture, sampler, and binding-slot values;
- fixed-size constant byte spans or caller-owned constant buffers;
- synthetic material fixture id values, slot capacities, status values, and
  snapshots;
- conversion or fill helpers that write caller-owned RenderCore fixture pass
  request values.

Public material fixture contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, Package ids, Streaming request ids,
  scene ids, UI ids, report handles, screenshot artifacts, visual-proof types,
  or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  material graph nodes, package archive internals, or original-game package
  layout details.

Public `YuRHI`, `YuResource`, `YuPackage`, `YuStreaming`, and `YuWorld` headers
must not gain material fixture dependencies for this gate.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts and bounded storage for a synthetic material binding
   fixture.
2. Use existing public RHI handles and landed RenderCore fixture request values
   as inputs; do not load Resources, parse packages, read files, decode assets,
   or compile shaders.
3. Validate a material binding and fill or update a caller-owned
   `RenderFixturePassRequest` with pipeline, sampled texture, sampler, and
   fixed constant metadata.
4. Keep public headers free of D3D11, DXGI, Win32, Platform, Resource, Package,
   Streaming, material graph, scene, UI, World, Script, and Game Adapter types.
5. Add deterministic fast tests for success, invalid inputs, capacity limits,
   duplicate material ids, no mutation on validation failure, and snapshot
   counters.
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
- discovery counts for `Material`, `RenderCore`, `RHI`, `Resource`,
  `Streaming`, `Upload`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no Material
  fixture tests;
- public-header scan for native/backend/Resource/Package/Streaming/scene/World/
  Game leakage;
- production dependency scan proving the material fixture does not depend on
  Package, File, Streaming, Resource load-state mutation, shader compiler,
  material graph, scene, UI, World, Script, Game Adapter, original-game parser,
  or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- RHI Null backend and RenderCore fixture request assertions through existing
  public APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  material fixture state, RenderCore pass state, or caller-owned output values.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of material fixture proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- shader compiler, material graph, scene traversal, UI, World, Script, or Game
  Adapter behavior as evidence for this gate.

## Non Goals

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

1. boundary review confirms the scope stays in a narrow material binding fixture
   over public RHI and RenderCore values without Resource/Streaming/scene
   ownership;
2. implementability review confirms the landed RenderCore fixture pass and
   existing public RHI binding contracts can support the first slice locally;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
