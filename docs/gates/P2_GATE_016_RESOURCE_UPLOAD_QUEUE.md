# P2-GATE-016: Resource Upload Queue

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NOT_APPROVED`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-014, P2-GATE-015, P1-GATE-006
Related decisions: ADR-0011, ADR-0013
Source baseline: `6e29663`
Proposal commit: pending

## Layer

L4-L5 lower-engine upload bridge over existing package/resource staging values
and public RHI value contracts.

This gate proposes the first bounded Resource upload queue that can consume
P2-GATE-015 package/resource staging completions, validate the destination
Resource handle/type, and call existing public `YuRHI` buffer or texture
creation/update APIs through caller-owned RHI device storage. It is not a
Resource load-state machine, package parser, image decoder, mesh decoder,
RenderCore pass, material system, scene streaming, UI, World, report,
screenshot, manual proof, or Game Adapter gate.

```text
PackageResourceStagingCompletion values
-> Resource handle/type validation values
-> public YuRHI buffer/texture descriptors and caller-owned device
-> bounded Resource upload queue records
-> later Resource load completion, RenderCore scheduling, material binding
```

## Current Reality

P2-GATE-015 landed a deterministic package/resource staging queue over
`YuPackage` load-plan values, `YuResource` handle/type validation values, and
`YuFile` async read completions. P2-GATE-008 and P2-GATE-014 provide public RHI
buffer, texture, sampler, update, and sampling value contracts. The engine still
has no Resource upload queue, no Resource load completion mutation, no asset
decode pipeline, no RenderCore pass scheduling, no material system, and no
scene-driven resource streaming.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `736`;
- `ctest --preset windows-fast-gate -N -L Resource`: `28`;
- `ctest --preset windows-fast-gate -N -L Package`: `34`;
- `ctest --preset windows-fast-gate -N -L File`: `26`;
- `ctest --preset windows-fast-gate -N -L AsyncIO`: `20`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `10`;
- `ctest --preset windows-fast-gate -N -L Upload`: `10`;
- `ctest --preset windows-fast-gate -N -L RHI`: `70`;
- `ctest --preset windows-fast-gate -N -L Fast`: `736`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `53`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `140`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Streaming`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Upload`: `0`.

## Owns

This gate owns the proposal for:

- a narrow upload bridge surface that depends on P2-GATE-015 staging
  completion values, existing `YuResource` validation values, and public
  `YuRHI` value contracts;
- value-only upload descriptors for resource handle, expected resource type,
  staging request id, upload kind, byte range, RHI buffer or texture descriptor,
  caller-owned RHI output handle storage, and caller-owned fence storage;
- bounded queue records for pending upload work and completion records;
- validation for null device, null output storage, stale Resource handles, type
  mismatch, failed staging completion, byte range overflow, empty upload bytes,
  unsupported upload kind, queue capacity overflow, completion capacity
  overflow, and duplicate upload ids;
- deterministic tests under default `windows-fast-gate` with `Resource`,
  `Streaming`, `Upload`, `RHI`, `Fast`, `PerformanceSmoke`, and
  `EvidenceOracle` labels;
- snapshot counters/statuses proving bounded queue behavior and no mutation on
  failed validation.

## Does Not Own

This gate does not own:

- Resource load completion mutation, ResourceRegistry storage ownership,
  acquire/release policy changes, dependency graph changes, or cache lifetime
  policy;
- package file parsing, original package format readers, compression, archive
  indexing, pack/rpack compatibility, or original-game output proof;
- image decode, texture import, audio decode, mesh decode, shader compile,
  material graph, or asset cache ownership;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  upload objects, hardware-only proof, or public native/backend leakage;
- command list recording, draw submission, RenderCore scheduling, material
  binding, scene traversal, UI, World, Script, gameplay, reports, screenshots,
  logs, sleeps, manual visual proof, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package owns metadata and deterministic entry/load-plan lookup;
- File owns path normalization, mounted reads, and async read completions;
- Resource owns identity, handle lifetime, dependency, and acquire/release
  vocabulary;
- the P2-GATE-015 staging bridge owns bounded package/resource staging records;
- this proposed upload bridge owns only bounded transfer from staging bytes into
  public RHI buffer or texture upload APIs;
- RHI owns backend-neutral GPU resource primitives and backend-private native
  objects;
- RenderCore and materials later own pass scheduling and bind policy;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep package metadata, file IO,
resource identity, decode, upload execution, render scheduling, and gameplay
meaning separate.

## Dependencies

Allowed dependencies:

- existing `YuStreaming` package/resource staging completion value contracts;
- existing `YuResource` public handle/type/status and const validation
  contracts;
- existing public `YuRHI` buffer, texture, fence, status, and device value
  contracts;
- a new narrow upload bridge surface in `YuStreaming` or an equivalent isolated
  lower-engine bridge module;
- `Tests/Streaming` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- Package parser implementations, original package readers, Resource load-state
  mutation, asset decode/import code, shader compiler/source tooling, RenderCore,
  material, scene, UI, World, Script, Game Adapter, reports, screenshots,
  visual proof tooling, manual proof, or original-game evidence;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Package, Resource, Streaming, or upload bridge headers;
- adding RHI dependencies to `YuResource` or `YuPackage` core;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public upload bridge contracts may expose value-only descriptors such as:

- `ResourceHandle` and expected `ResourceTypeId` values for validation;
- staging request id, byte offset, byte size, and staging completion status;
- upload kind values for buffer or texture upload;
- public `YuRHI` buffer/texture descriptors, handles, fence handles, and status
  values only inside the upload bridge boundary;
- fixed-capacity queue and completion snapshots.

Public upload bridge contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, material ids,
  scene ids, UI ids, report handles, screenshot artifacts, visual-proof types,
  or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  package archive internals, or original-game package layout details.

Public `YuResource` and `YuPackage` headers must remain free of RHI, Streaming,
File, D3D11, DXGI, Platform, RenderCore, material, scene, UI, World, Script, and
Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts and bounded queue storage for Resource upload
   requests and completions.
2. Consume existing package/resource staging completion bytes and status values;
   do not parse package files or original package formats.
3. Use existing Resource handle/type validation without mutating Resource
   acquire/release, dependency, cache, or load state.
4. Use existing public RHI buffer/texture creation or update APIs through a
   caller-provided `IRhiDevice` reference or pointer.
5. Add deterministic fast tests for buffer upload success, texture upload
   success, invalid Resource handle, type mismatch, failed staging completion,
   empty bytes, byte range overflow, null RHI device, null output storage,
   queue overflow, completion overflow, duplicate upload ids, and snapshot
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
- discovery counts for `Resource`, `Streaming`, `Upload`, `RHI`, `Fast`,
  `PerformanceSmoke`, `EvidenceOracle`, and `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no Resource
  upload tests;
- public-header scan for native/backend/RenderCore/World/Game leakage;
- production dependency scan proving `YuResource` and `YuPackage` do not gain
  RHI, File, Streaming, RenderCore, material, scene, UI, World, Script, Game
  Adapter, or original-game parser/decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- RHI Null backend upload assertions through existing public `IRhiDevice`
  buffer/texture APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource state, upload queue state, or RHI output handles.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of Resource upload proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- RenderCore draw, material bind, scene streaming, or Game Adapter behavior as
  evidence for this gate.

## Non Goals

- No package parser or original package reader.
- No Resource load completion state machine.
- No image, audio, mesh, shader, or material decoder.
- No RenderCore pass scheduling.
- No material binding.
- No scene traversal or World streaming.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in a narrow upload bridge plus
   public Resource, Streaming, and RHI values without mutating Package or
   Resource core ownership;
2. implementability review confirms existing staging completions, Resource
   validation, and public RHI upload contracts can support the first slice
   locally;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
