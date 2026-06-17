# P2-GATE-014: Texture Sampling Fixture

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-009, P2-GATE-013, ENG-096
Related decisions: ADR-0011
Source baseline: `1ee9fa4`
Proposal commit: `d096ffa`
Amendment commit: `f832109`

## Layer

L3 lower-engine graphics backend proof.

This gate proposes the first texture-sampling fixture after the landed D3D11
resource/pipeline primitives, visible triangle, and indexed static mesh slices.
It is not a Resource, Package, File IO, image decode, material, RenderCore,
scene, UI, World, report, screenshot, manual visual proof, shader compiler, or
Game Adapter gate. The goal is to prove that caller-owned texture bytes and
sampler state can be bound through `YuRHI` and sampled by the private D3D11
backend, with proof based on capture bytes and bounded counters/statuses.

```text
caller-owned rgba texture bytes
-> backend-neutral RHI sampled-texture binding values
-> bounded Null RHI validation and counters
-> private D3D11 shader-resource view and sampler binding
-> deterministic textured geometry hardware-smoke proof if admitted
-> later Resource texture loading, material system, RenderCore scheduling
```

## Current Reality

P2-GATE-008 landed backend-neutral texture and sampler primitive lifecycle
contracts, plus private D3D11 creation/update/destroy coverage. P2-GATE-009
landed a visible triangle proof through RHI capture bytes. P2-GATE-013 landed
indexed static geometry through `YuRHI` value contracts and private D3D11
indexed draw. The engine still has no texture-sampling command contract, no RHI
shader-resource binding value, no material texture slot, no image decoder, no
Resource texture asset identity, no RenderCore pass scheduling, no scene-driven
texture use, and no Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `718`;
- `ctest --preset windows-fast-gate -N -L RHI`: `62`;
- `ctest --preset windows-fast-gate -N -L Fast`: `718`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `49`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `123`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-fast-gate -N -L TextureSampling`: `0`;
- `ctest --preset windows-fast-gate -N -L Sampler`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `6`;
- `ctest --preset windows-hardware-smoke -N -L D3D11`: `4`;
- `ctest --preset windows-hardware-smoke -N -L TextureSampling`: `0`.

## Approval Evidence

Approved after:

- ENG-119A boundary and quality review found a queue consistency blocker;
- ENG-119A2 boundary and quality rerun PASS after fix commit `f832109`;
- ENG-119B implementability review PASS with no `NEEDS_IMPLEMENTABILITY`
  blocker;
- ENG-119C test and preset review PASS on proposal commit `d096ffa`;
- ENG-119C2 test-policy rerun PASS after the TextureSampling label consistency
  fix.

Review evidence:

- proposal commit `d096ffaea529b13494020e8164399dcc04c246e8` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- amendment commit `f832109ebb9fdb61558b7c3edf9a01e692a77d23` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check d096ffa^ d096ffa` passed;
- `git diff --check f832109^ f832109` passed;
- review worktrees stayed clean and reviewers made no source, doc, commit, or
  push changes;
- baseline discovery after the amendment is default `718`, `RHI` `62`, `Fast`
  `718`, `PerformanceSmoke` `49`, and `EvidenceOracle` `123`;
- default `HardwareSmoke`, `D3D11`, `Win32`, `TextureSampling`, and `Sampler`
  discovery remains `0`;
- `windows-hardware-smoke` discovers `6`, with `RHI` `4`, `D3D11` `4`,
  `Win32` `6`, `HardwareSmoke` `6`, and no texture-sampling hardware test
  admitted yet.

Approval conditions:

- implementation must keep public `YuRHI` headers free of Windows, D3D11, DXGI,
  COM, Platform, RenderCore, Resource, Package, World, UI, report, screenshot,
  visual-proof, and Game Adapter types;
- implementation must keep default `windows-fast-gate` deterministic and
  no-real-device;
- texture/sampler binding and sampling proof must stay inside `YuRHI`, private
  D3D11, `Tests/Rhi`, and root CMake labels;
- optional hardware-smoke proof must be isolated in `windows-hardware-smoke`
  with `HardwareSmoke`, `RHI`, `D3D11`, `Win32`, `TextureSampling`, and
  `Sampler` labels;
- proof must use capture bytes plus bounded counters/statuses, not reports,
  screenshots, logs, sleeps, manual visual inspection, source tooling, file
  proof, original-game output, or silent skip.

## Owns

This gate owns the proposal for:

- backend-neutral sampled-texture binding value contracts using existing
  `RhiTextureHandle` and `RhiSamplerHandle` values;
- bounded texture/sampler bind command records in `RhiCommandList`;
- Null RHI validation for bind ordering, missing texture, missing sampler,
  stale handles, slot overflow, and bounded snapshot counters;
- private D3D11 shader-resource view and sampler binding for already-created
  RHI texture/sampler primitives;
- deterministic RHI tests for value defaults, validation failures, snapshot
  counters, command capacity, and dependency boundaries;
- optional D3D11 hardware-smoke proof that samples a tiny caller-owned RGBA
  texture and validates capture bytes.

## Does Not Own

This gate does not own:

- image files, image decode, texture import, Resource texture assets, Package
  streaming, File IO, async upload queues, texture cache ownership, asset
  identity, or material binding;
- RenderCore pass scheduling, render graph, material system, shader parameter
  tables, scene traversal, camera, batching, instancing, indirect draws,
  compute, skinning, animation, or lighting;
- shader compiler, shader source tooling, material graph, texture atlases,
  mip generation, anisotropy policy, color-management policy, depth/stencil,
  blending expansion, or descriptor heap abstraction;
- reports, screenshots, manual visual proof, visual inspection, UI, World,
  Script, gameplay, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- RHI owns GPU resource handles, texture/sampler binding commands, backend
  shader-resource state, present, capture, validation counters, and backend
  synchronization details;
- Resource later owns texture asset identity, decoded image data, dependency
  lifetime, and streaming ownership;
- RenderCore later owns pass scheduling, material parameter binding, draw
  ordering, and scene-to-draw translation;
- World and Game Adapter later own gameplay scene meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate only uses those engines to keep texture sampling mechanics,
asset loading, material policy, render scheduling, and gameplay scene meaning
separate.

## Dependencies

Allowed dependencies:

- existing `YuRHI` public and private files;
- existing RHI D3D11 private backend files;
- existing `Tests/Rhi` deterministic and hardware-smoke fixtures;
- root CMake/CTest labels and optional `windows-hardware-smoke` admission;
- this gate and queue documentation.

Forbidden dependencies:

- production Resource, Package, File, RenderCore, UI, World, Script, Game
  Adapter, report, screenshot, visual proof, original-game evidence, shader
  compiler, shader source tooling, image decoding, texture importer, material
  graph, or texture asset code;
- Windows SDK, D3D11, DXGI, COM, Platform, RenderCore, Resource, Package,
  World, UI, report, screenshot, visual-proof, or Game Adapter types in public
  `YuRHI` headers;
- generated screenshots, generated reports, manual visual inspection, sleep
  timing, logs, or original-game output as evidence.

## Public Contract Boundary

Public `YuRHI` may expose value-only contracts such as:

- sampled texture binding descriptors with `RhiTextureHandle`,
  `RhiSamplerHandle`, and bounded slot indices;
- bind-texture or bind-sampler command methods on `RhiCommandList`;
- snapshot counters/statuses for sampled texture binds and rejection paths.

Public `YuRHI` must not expose:

- `ID3D11*`, `IDXGI*`, `IUnknown`, `HWND`, `HANDLE`, Windows headers, COM
  ownership, shader-resource-view handles, native sampler pointers, or backend
  storage layout;
- Resource asset IDs, Package paths, File paths, image-decoder outputs,
  material names, RenderCore pass IDs, scene IDs, UI IDs, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only the value contracts required to bind an existing RHI texture and
   sampler to a bounded shader-resource slot.
2. Extend command records and snapshots for sampled texture binding without
   unbounded command storage growth.
3. Extend Null RHI validation for ordering, stale handles, range overflow,
   capacity, and snapshot counters.
4. Extend private D3D11 command replay with shader-resource-view and sampler
   binding for already-created texture/sampler primitives.
5. Add deterministic `Tests/Rhi` coverage under default `windows-fast-gate`.
6. Optionally admit one `windows-hardware-smoke` test with
   `HardwareSmoke`, `RHI`, `D3D11`, `Win32`, `TextureSampling`, and `Sampler`
   labels.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require a
real GPU device. Hardware-smoke coverage, if admitted, must remain isolated to
`windows-hardware-smoke`.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- discovery counts for `RHI`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`,
  `HardwareSmoke`, `D3D11`, `Win32`, `TextureSampling`, and `Sampler`;
- `windows-hardware-smoke` discovery and execution if a texture-sampling
  hardware-smoke test is admitted;
- public-header scan for backend/native/resource/render/game leakage;
- production dependency scan for forbidden upper modules;
- proof-shape scan rejecting reports, screenshots, logs, sleeps, manual visual
  proof, silent skip, file proof, and original-game evidence;
- changed-path and `CMakePresets.json` no-drift checks.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- capture-byte assertions in optional hardware-smoke tests;
- explicit unavailable-device skip only for unrelated hardware that is already
  optional in the preset.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, or original-game output;
- silent skip of texture-sampling proof after admitting a texture-sampling
  hardware-smoke test.

## Non Goals

- No Resource texture loader.
- No File or Package texture read path.
- No image decoder.
- No material system.
- No RenderCore pass scheduling.
- No shader compiler or shader source tooling.
- No UI, World, Script, gameplay, report, screenshot, or Game Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays inside `YuRHI`, private D3D11,
   `Tests/Rhi`, root CMake labels, and docs;
2. implementability review confirms the existing texture/sampler primitive and
   static mesh fixture foundations can support this slice locally;
3. test-policy review confirms default fast-gate isolation, hardware-smoke
   labels, and capture-byte/counter proof requirements.
