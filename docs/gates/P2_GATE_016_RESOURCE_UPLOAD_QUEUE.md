# P2-GATE-016: Resource Upload Queue

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-008, P2-GATE-014, P2-GATE-015, P1-GATE-006
Related decisions: ADR-0011, ADR-0013
Source baseline: `6e29663`
Proposal commit: `141b36d`

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

## Approval Evidence

Approved after:

- ENG-123A boundary and quality review PASS with no `NEEDS_ARCHITECTURE`
  blocker;
- ENG-123B implementability review PASS with no `NEEDS_IMPLEMENTABILITY`
  blocker;
- ENG-123C test and preset review PASS with no `NEEDS_TEST_POLICY` blocker.

Review evidence:

- proposal commit `141b36d9b53bda182ad87aeefbb267853e9e3903` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- review worktrees stayed clean and reviewers made no source, doc, commit, or
  push changes;
- `git diff --check 141b36d^ 141b36d` passed;
- baseline discovery is default `736`, `Resource` `28`, `Package` `34`,
  `File` `26`, `AsyncIO` `20`, `Streaming` `10`, `Upload` `10`, `RHI` `70`,
  `Fast` `736`, `PerformanceSmoke` `53`, `EvidenceOracle` `140`, and
  `HardwareSmoke` `0`;
- `windows-hardware-smoke` discovers `7`, with `Streaming` and `Upload`
  remaining `0`.

Approval conditions:

- implementation must remain a narrow Resource upload bridge over P2-GATE-015
  staging completions, existing Resource validation, and public `YuRHI`
  buffer/texture creation or update value APIs;
- upload completions must remain value/status records and must not mutate
  Resource load completion state;
- `YuResource` and `YuPackage` core must not gain RHI, File, Streaming,
  RenderCore, material, scene, UI, World, Script, Game Adapter, parser, or
  decode ownership;
- default `windows-fast-gate` must remain deterministic and no-real-device;
- no hardware-smoke admission is expected for this gate;
- proof must use deterministic value/counter/status assertions through public
  RHI Null backend upload behavior;
- proof must reject screenshots, reports, generated artifacts, logs, sleeps,
  manual visual proof, original-game packages/output, silent skip, RenderCore
  draw, material bind, scene streaming, or Game Adapter behavior.

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

## L0-RES-004 Evidence Sync

L0-RES-004 Resource residency/upload chain closure is PASS at
`45f91f6cda02e42f0dce7eae7ff3df6db3616467` for this gate's upload queue
surface. Focused QA task `2917323c-9869-4a1c-a9fb-67a90b513a23` reports
`YuStreamingTests` and `YuResourceTests` build PASS,
`Streaming_ResourceUpload_` discovery/execution `17/17` PASS,
`Streaming_ResourceUploadCommit_` discovery/execution `9/9` PASS,
`Resource_LoadCommit_`/`Resource_Residency_` discovery/execution `18/18` PASS,
combined focused execution `44/44` PASS, and a clean read-only QA workspace.
Readiness task `d88846fd` records existing Resource/Streaming/RHI value/status
records for upload queue, upload commit, Resource load commit, residency
budget/state, pin/unpin/eviction, and stale/invalid handle no-mutation.

This sync keeps upload queue evidence fixture-bound and deterministic. QA did
not build or execute `YuRHITests`, RHI 38-row dependency execution,
adjacent/full Resource, full `^Resource_`, or broad/full CTest. RHI primitive
evidence remains a dependency note, and `L0-RHI-003` is not table-closed by
this gate update.

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
