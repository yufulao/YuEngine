# P2-GATE-026: Resource Decode Result Import-Ready Record

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Combined lower-engine review
Depends on: P2-GATE-024, P2-GATE-023, P2-GATE-022, P2-GATE-021, P1-GATE-006
Related decisions: ADR-0013
Source baseline: `7a620af`
Proposal commit: `74189da`
Landed commit: `5d28e38`
Approval evidence: ENG-143CR combined proposal review PASS.

## Layer

L4-L5 lower-engine Resource import-ready decode result boundary over landed
Resource cache payload ownership and Resource decode-plan records.

This gate proposes the first bounded Resource-owned import-ready decode result
record. The first slice may link an existing Resource decode plan to a
caller-provided decoded-result metadata value, validate fixed result id, asset
class, decoded byte count, capacity, and budget, and expose deterministic
counters. It does not run a real codec, store decoded bytes, create texture,
audio, mesh, shader, or material objects, upload to RHI, schedule RenderCore
work, touch scene/UI/World/Script, or define Game Adapter behavior.

```text
landed Resource cache payload bytes
-> landed Resource decode plan metadata
-> Resource-owned import-ready decode result record
-> later real texture/audio/mesh import and RHI upload
```

## Current Reality

P2-GATE-021 landed Resource upload completion commit at `475c371`.
P2-GATE-022 landed Resource residency budget policy at `d2f2059`.
P2-GATE-023 landed Resource cache payload ownership at `aca6170`.
P2-GATE-024 landed Resource asset decode-plan records at `a6fbabf`.
Resource can now own uploaded/resident Resource state, opaque cached payload
bytes, and decode-plan metadata. It still has no import-ready decoded-result
record, no decoded byte ownership, no real codec, no decoded object lifetime,
no RHI upload execution, no RenderCore scheduling, no scene streaming, and no
Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `858`;
- `ctest --preset windows-fast-gate -N -L Resource`: `96`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `36`;
- `ctest --preset windows-fast-gate -N -L Upload`: `43`;
- `ctest --preset windows-fast-gate -N -L RHI`: `141`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `54`;
- `ctest --preset windows-fast-gate -N -L Material`: `40`;
- `ctest --preset windows-fast-gate -N -L Fast`: `858`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `91`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `262`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Resource`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Streaming`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Upload`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Material`: `0`.

## Approval Evidence

Approved after ENG-143CR combined proposal review PASS.

Review evidence:

- proposal commit `74189dabe898917a536120265e9b7add0858b634` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 74189da^..74189da` passed;
- review worktree stayed clean at writeback and the reviewer made no source,
  doc, commit, push, approval, or implementation changes;
- review confirmed no `NEEDS_ARCHITECTURE`, `NEEDS_IMPLEMENTABILITY`, or
  `NEEDS_TEST_POLICY` blocker;
- boundary review confirmed the proposal stays in Resource-owned import-ready
  decode-result metadata over landed decode-plan records;
- implementability review confirmed landed Resource decode-plan fields and
  ResourceRegistry lifecycle hooks are sufficient for the first slice;
- test-policy review confirmed deterministic default `windows-fast-gate`
  evidence, focused Resource decode-result tests, label discovery,
  CMakePresets no-drift, dependency scans, and proof-shape scans are required;
- proposal discovery counts matched the reviewed baseline: default fast gate
  `858`, `Resource` `96`, `Streaming` `36`, `Upload` `43`, `RHI` `141`,
  `RenderCore` `54`, `Material` `40`, `Fast` `858`, `PerformanceSmoke` `91`,
  `EvidenceOracle` `262`, default `HardwareSmoke` `0`, and
  `windows-hardware-smoke` `7`.

## Owns

This gate owns the proposal for:

- Resource-owned decode-result status, operation, request, record, result class,
  and snapshot value contracts;
- fixed-capacity Resource-owned import-ready result records that reference an
  active decode plan record without copying or storing decoded bytes;
- validation for Resource handle, generation, type, uploaded state, resident
  state, cache payload id, decode plan id, decode result id, asset class,
  decoded byte count, duplicate result id, record capacity, and result budget;
- query and release behavior for Resource-owned import-ready result metadata;
- deterministic snapshots for accepted, rejected, capacity rejected, budget
  rejected, query, and release outcomes;
- lifecycle proof that releasing a decode plan or its cache payload clears only
  dependent Resource-owned decode-result records;
- deterministic tests under default `windows-fast-gate` with `Resource`,
  `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- real image decode, audio decode, mesh decode, shader compile, material graph,
  asset database, editor assets, decoded asset object lifetime, cache file
  persistence, or decoded byte storage;
- package file parsing, original package readers, compression, archive indexing,
  pack/rpack compatibility, original-game output proof, or File IO expansion;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific RHI
  objects, RHI resource creation, RHI upload execution, RHI resource
  destruction, or hardware-only proof;
- RenderCore pass scheduling, render graph execution, frame graph execution,
  renderer scheduling, material binding policy, scene traversal, UI, World,
  Script, gameplay, reports, screenshots, logs, sleeps, manual visual proof, or
  Game Adapter behavior;
- asynchronous streaming policy, background IO, dependency loading policy,
  cache compression, virtual file mounting, or real asset format ownership.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package owns deterministic entry/load-plan lookup;
- File owns path normalization, mounted reads, and async read completions;
- Streaming owns bounded staging and upload bridges;
- Resource owns identity, handle lifetime, load state, residency, cache payload
  ownership, decode-plan metadata, and this proposed import-ready result
  metadata boundary;
- real asset decode/import later owns interpreting texture, audio, mesh, shader,
  or material bytes and producing decoded objects or upload payloads;
- RHI owns backend-neutral GPU resource primitives and backend-private native
  objects;
- RenderCore and materials later own scheduling and bind policy;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep cache bytes, decode planning,
decode-result metadata, real asset import, render scheduling, scene meaning, and
gameplay meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuResource` handle, type, load-state, registry, residency,
  cache payload, decode plan, and snapshot value contracts;
- new Resource-owned decode-result status, request, record, operation, result
  class, and snapshot value contracts;
- active Resource decode-plan records from P2-GATE-024;
- fixed caller-provided metadata values that describe an import-ready decoded
  result without storing decoded bytes;
- `Tests/Resource` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuStreaming`, `YuRHI`, `YuPackage`, `YuFile`, `YuRenderCore`, Material,
  World, Script, UI, Game Adapter, parser, real decoder, report, screenshot,
  visual proof, or original-game evidence dependencies from Resource core;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Resource decode-result headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public Resource contracts may expose value-only descriptors such as:

- Resource decode-result status and operation values;
- Resource handle, expected Resource type, cache payload id, decode plan id,
  decode result id, asset class, result class, decoded byte count, and record
  slot values;
- bounded snapshots for decode-result counters and last operation;
- query functions that return import-ready result metadata without exposing
  decoded bytes, decoded object pointers, parser state, or backing byte
  pointers.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, material graph
  nodes, scene ids, UI ids, report handles, screenshot artifacts, visual-proof
  types, or Game Adapter types;
- real decoded image/audio/mesh payload formats, shader source/compiler
  handles, package archive internals, parser objects, cache file paths, decoded
  asset object pointers, or original-game package layout details.

Public `YuResource` headers must remain free of Streaming, RHI, Package, File,
D3D11, DXGI, Platform, RenderCore, material graph, scene, UI, World, Script, and
Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Resource-owned value contracts for decode-result status, operation,
   request, record, result class, and snapshot.
2. Add bounded ResourceRegistry storage for import-ready result records that
   reference existing decode-plan records without storing decoded bytes.
3. Validate Resource handle, generation, type, upload state, residency state,
   cache payload id, active decode plan id, decode result id, asset class,
   decoded byte count, duplicate result id, record capacity, and budget before
   mutation.
4. Reject missing decode plan, stale handle, type mismatch, not resident, failed
   load, asset class mismatch, decoded byte mismatch, duplicate result id,
   capacity overflow, and budget overflow without mutating Resource slot,
   residency, cache payload, decode plan, Streaming, Package, RHI, RenderCore,
   or upper-engine state.
5. Add query and release behavior that returns or clears only Resource-owned
   decode-result metadata.
6. Clear dependent decode-result records when a decode plan or cache payload is
   released, without changing load or residency state.
7. Keep Resource core free of Streaming, RHI, Package, File, RenderCore,
   material graph, render graph, scene, UI, World, Script, and Game Adapter
   dependencies.
8. Add deterministic fast tests for decode-result success from an active decode
   plan, metadata query, release, missing plan, stale handle, type mismatch, not
   resident, failed load, asset class mismatch, decoded byte mismatch, duplicate
   result id, capacity overflow, budget overflow, dependent clear, and no
   mutation on failed validation.
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
- focused `Resource_DecodeResult` or equivalent tests proving the new boundary;
- discovery counts for `Resource`, `Streaming`, `Upload`, `RHI`, `RenderCore`,
  `Material`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no Resource
  decode-result tests;
- public-header scan for native/backend/RenderCore/World/Game leakage;
- production dependency scan proving `YuResource` does not gain Streaming, RHI,
  Package, File, RenderCore, material graph, render graph, frame graph, scene,
  UI, World, Script, Game Adapter, original-game parser, or real decode
  dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic assertions over active decode-plan records and caller-provided
  import-ready metadata values;
- ResourceRegistry decode-result state and counter assertions through
  Resource-owned public APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource decode-result state, decode-plan state, cache payload state,
  residency state, load commit state, Streaming state, Package state, RHI state,
  or RenderCore state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of Resource decode-result proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- File reads, package parsing, real image/audio/mesh decode, texture/audio/mesh
  import, decoded byte storage, RHI upload, RenderCore draw, material graph,
  render graph, frame graph, scene streaming, UI, World, Script, or Game Adapter
  behavior as evidence for this gate.

## L0-RES-003 Evidence Sync

L0-RES-003 records the Resource cache/decode chain as PASS at
`origin/main@8c3a200d813173efe1607e594777afd6f029cc7c`. Readiness task
`ba6025e8` accepted the existing Resource cache payload, decode plan, decode
result, decoded payload ownership, release/dependent clear, budget/capacity,
and no-mutation value/status records as the baseline.

The initial focused QA task `085247f3` isolated one decoded-payload capacity
status-order issue in `StoreDecodedPayload`. Implementation task `abfdb2d1`
landed the narrow `ResourceRegistry.cpp` fix at `8c3a200d` so physical decoded
payload record capacity is checked before reference budget. Focused QA task
`ca5c3c1b-e61a-4095-8e3c-2e0dfccc2b40` then reported `YuResourceTests` build
PASS, exact `Resource_DecodedPayload_RejectsCapacityOverflow` `1/1` PASS,
focused cache/decode discovery and execution `65/65` PASS, clean read-only QA,
and no adjacent/full Resource or broad/full CTest.

This evidence keeps decode-result records as Resource import-ready metadata. It
does not complete Resource residency/upload, RuntimeAsset bridge or CMake
cross-proof, RenderScene/RHI, World/editor/importer, old-package compatibility,
real codec/parser behavior, unrelated animation mapping, Package/Resource
public API expansion, adjacent/full Resource rows, or broad/full CTest.

## Non Goals

- No package parser or original package reader.
- No File IO or mounted archive read behavior.
- No real image, audio, mesh, shader, or material decoder.
- No decoded byte storage, decoded object lifetime, or cache-file persistence.
- No RHI upload execution or GPU memory ownership.
- No RenderCore pass scheduling, render graph execution, or frame graph
  execution.
- No material graph.
- No scene traversal or World streaming.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after a combined lower-engine review
confirms the proposal stays in Resource-owned import-ready result metadata over
landed decode-plan records, remains implementable with the landed
ResourceRegistry cache payload and decode-plan paths, and preserves deterministic
fast-gate proof without native/backend, Resource-to-RHI upload, RenderCore,
scene, report, screenshot, log, sleep, manual, original-game, or hardware-only
evidence.
