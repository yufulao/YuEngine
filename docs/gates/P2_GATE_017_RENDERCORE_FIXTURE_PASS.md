# P2-GATE-017: RenderCore Fixture Pass

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-013, P2-GATE-014, P2-GATE-016
Related decisions: ADR-0011
Source baseline: `55af599`
Proposal commit: `c551520`

## Layer

L5 lower-engine RenderCore fixture boundary over existing public RHI value
contracts.

This gate proposes the first bounded RenderCore fixture pass. The pass can
accept caller-owned RHI target, pipeline, vertex/index buffer, texture, sampler,
draw, submit, present, capture, and snapshot values, then produce deterministic
pass status and counter evidence through public `YuRHI` APIs. It is not a
material system, scene renderer, Resource loader, Streaming owner, shader
compiler, screenshot/report proof, UI, World, or Game Adapter gate.

```text
caller-owned public YuRHI handles and descriptors
-> bounded RenderCore fixture pass descriptor
-> public YuRHI command recording, submit, present, capture values
-> later materials, scene traversal, renderer scheduling, UI, World
```

## Current Reality

P2-GATE-008 landed RHI buffer/texture/sampler/shader/pipeline/fence value
contracts. P2-GATE-013 and P2-GATE-014 landed static mesh and texture sampling
fixtures through RHI/D3D11 value contracts. P2-GATE-016 landed Resource upload
queue proof into public RHI buffer/texture value APIs. The engine still has no
RenderCore module, no render pass descriptor, no pass scheduling, no material
system, no scene traversal, no renderer-owned resource binding, and no
World/Game Adapter render policy.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `753`;
- `ctest --preset windows-fast-gate -N -L Resource`: `45`;
- `ctest --preset windows-fast-gate -N -L Package`: `34`;
- `ctest --preset windows-fast-gate -N -L File`: `26`;
- `ctest --preset windows-fast-gate -N -L AsyncIO`: `20`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `27`;
- `ctest --preset windows-fast-gate -N -L Upload`: `27`;
- `ctest --preset windows-fast-gate -N -L RHI`: `87`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `0`;
- `ctest --preset windows-fast-gate -N -L Fast`: `753`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `56`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `157`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`;
- `ctest --preset windows-hardware-smoke -N -L Upload`: `0`.

## Owns

This gate owns the proposal for:

- a narrow RenderCore fixture pass surface that depends only on public `YuRHI`
  value contracts and caller-owned RHI handles;
- value-only fixture descriptors for target handle, pipeline handle,
  vertex/index buffer views, sampled texture binding, sampler binding, indexed
  draw descriptor, clear color, capture byte budget, and caller-owned output
  storage;
- bounded pass execution records and completion/status snapshots;
- validation for null RHI device, invalid target, invalid pipeline, missing
  vertex buffer, missing index buffer, invalid draw descriptor, invalid texture
  binding, invalid sampler binding, insufficient capture storage, command list
  overflow, and failed RHI status propagation;
- deterministic tests under default `windows-fast-gate` with `RenderCore`,
  `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels;
- snapshot counters/statuses proving bounded pass behavior and no mutation on
  failed validation.

## Does Not Own

This gate does not own:

- Resource load-state mutation, ResourceRegistry storage ownership, package
  parsing, file IO, Streaming ownership, upload queue ownership, or cache
  lifetime policy;
- image decode, texture import, audio decode, mesh decode, shader compile,
  shader source tooling, material graph, material binding policy, or asset cache
  ownership;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  render objects, hardware-only proof, or public native/backend leakage;
- scene traversal, renderer frame graph, visibility culling, batching,
  animation, UI, World, Script, gameplay, reports, screenshots, logs, sleeps,
  manual visual proof, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- RHI owns backend-neutral GPU resource primitives, command recording, submit,
  present, capture, and backend-private native objects;
- Resource, Package, File, and Streaming own asset identity, package/file
  staging, and upload preparation outside RenderCore;
- this proposed RenderCore fixture pass owns only a bounded, synthetic pass over
  caller-provided RHI values;
- materials later own shader/material binding policy;
- scene and World later own object traversal, visibility, transform, and
  gameplay meaning;
- Game Adapter later owns title-specific render decisions.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep RHI primitives, resource
loading, render pass scheduling, material policy, scene traversal, and gameplay
meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuRHI` command, resource handle, binding, draw, submit,
  present, capture, status, and snapshot value contracts;
- a new narrow `YuRenderCore` module or equivalent isolated lower-engine
  fixture pass surface;
- `Tests/RenderCore` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- Package, File, Streaming, Resource load-state mutation, package parser
  implementations, original package readers, asset decode/import code, shader
  compiler/source tooling, material, scene, UI, World, Script, Game Adapter,
  reports, screenshots, visual proof tooling, manual proof, or original-game
  evidence;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public RenderCore headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public RenderCore fixture contracts may expose value-only descriptors such as:

- public `YuRHI` target, pipeline, buffer, texture, sampler, command list,
  draw, status, and snapshot values;
- clear color, capture byte budget, fixture pass id, fixed-capacity limits, and
  pass completion status values;
- caller-owned output storage pointers or references where existing local
  patterns require them.

Public RenderCore fixture contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, Resource handles, Package ids,
  Streaming request ids, material ids, scene ids, UI ids, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  package archive internals, or original-game package layout details.

Public `YuRHI`, `YuResource`, `YuPackage`, and `YuStreaming` headers must not
gain RenderCore dependencies for this gate.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts and bounded pass storage for a synthetic RenderCore
   fixture pass.
2. Use existing public RHI handles and descriptors as input; do not load
   Resources, parse packages, read files, or decode assets.
3. Record a deterministic command sequence through public `IRhiDevice`
   operations such as clear, bind pipeline, bind vertex/index buffer, bind
   sampled texture, bind sampler, draw indexed, submit, present, and capture.
4. Keep public RenderCore headers free of D3D11, DXGI, Win32, Platform,
   Resource, Package, Streaming, material, scene, UI, World, Script, and Game
   Adapter types.
5. Add deterministic fast tests for success, invalid inputs, RHI failure
   propagation, command/capture capacity limits, no mutation on validation
   failure, and snapshot counters.
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
- discovery counts for `RenderCore`, `RHI`, `Resource`, `Streaming`, `Upload`,
  `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no RenderCore
  fixture tests;
- public-header scan for native/backend/Resource/Package/Streaming/material/
  scene/World/Game leakage;
- production dependency scan proving RenderCore does not depend on Package,
  File, Streaming, Resource load-state mutation, material, scene, UI, World,
  Script, Game Adapter, original-game parser, or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- RHI Null backend command/capture assertions through existing public
  `IRhiDevice` APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  RenderCore pass state or RHI output handles.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of RenderCore fixture proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- material bind, scene traversal, UI, World, Script, or Game Adapter behavior as
  evidence for this gate.

## Non Goals

- No Resource load completion state machine.
- No Package/File/Streaming ownership.
- No image, audio, mesh, shader, or material decoder.
- No shader compiler or shader source tooling.
- No material system.
- No scene traversal or World rendering.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in a narrow RenderCore fixture pass
   over public RHI values without Resource/Streaming/material/scene ownership;
2. implementability review confirms existing public RHI command and resource
   contracts can support the first slice locally;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
