# P2-GATE-023: Resource Cache Payload Ownership

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-022, P2-GATE-021, P1-GATE-006
Related decisions: ADR-0013
Source baseline: `2323112`
Proposal commit: `f97a4a9`
Landed commit: `aca6170`

## Layer

L4-L5 lower-engine Resource cache payload ownership over landed Resource
identity, upload completion commit state, and Resource residency budget policy.

This gate proposes the first bounded Resource-owned cache payload store. It can
accept caller-provided opaque bytes for already uploaded and resident Resource
slots, copy those bytes into fixed-capacity Resource-owned cache records, and
report deterministic ownership, capacity, and release counters. It does not read
files, parse packages, decode assets, import textures/audio/meshes, own RHI
objects, destroy RHI resources, schedule RenderCore work, traverse scenes, drive
UI, touch World, run Script, or define Game Adapter behavior.

```text
ResourceUploadCompletion values
-> Resource-owned load commit state
-> Resource-owned residency state and budget policy
-> Resource-owned opaque cache payload bytes and records
-> later asset decode/import, render graph, scene streaming
```

## Current Reality

P2-GATE-015 landed a package/resource staging queue at `6e29663`. P2-GATE-016
landed a Resource upload queue at `55af599`. P2-GATE-021 landed a Resource
upload completion commit bridge at `475c371`. P2-GATE-022 landed a Resource
residency budget policy at `d2f2059`. P2-GATE-023 landed Resource cache payload
ownership at `aca6170`. `YuResource` can now track terminal upload load state,
residency state, budget counters, pin/unpin, deterministic eviction candidates,
and Resource-owned opaque cache payload records, but it still has no
decode/import pipeline, no render graph, no scene streaming, and no Game Adapter
behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `820`;
- `ctest --preset windows-fast-gate -N -L Resource`: `72`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `36`;
- `ctest --preset windows-fast-gate -N -L Upload`: `43`;
- `ctest --preset windows-fast-gate -N -L RHI`: `127`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `40`;
- `ctest --preset windows-fast-gate -N -L Material`: `26`;
- `ctest --preset windows-fast-gate -N -L Fast`: `820`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `78`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `224`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Resource`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Streaming`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Upload`: `0`;
- `ctest --preset windows-hardware-smoke -N -L RHI`: `5`;
- `ctest --preset windows-hardware-smoke -N -L RenderCore`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Material`: `0`.

## Owns

This gate owns the proposal for:

- Resource-owned cache payload status values for success, invalid handle,
  generation mismatch, type mismatch, not uploaded, not resident, pinned where
  release requires rejection, duplicate payload id, capacity exceeded, budget
  exceeded, empty payload, and invalid argument;
- Resource-owned cache payload descriptors containing Resource handle, expected
  Resource type, payload id, payload byte count, and fixed cache slot metadata;
- fixed-capacity Resource-owned opaque byte storage for caller-provided payload
  bytes without decoding or interpreting the payload format;
- ResourceRegistry mutation APIs that store payload bytes only for uploaded and
  resident Resource slots;
- ResourceRegistry release APIs that clear only Resource-owned cache payload
  state and counters without changing load commit, residency, Streaming, RHI, or
  Package state;
- fixed-capacity cache payload records and snapshots for stored, read, released,
  rejected, duplicate, capacity-rejected, and budget-rejected outcomes;
- deterministic tests under default `windows-fast-gate` with `Resource`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels;
- proof that failed validation does not mutate Resource slot state, residency
  state, load commit state, Package state, Streaming queues, RHI state, or any
  upper-engine binding.

## Does Not Own

This gate does not own:

- package file parsing, original package readers, compression, archive indexing,
  pack/rpack compatibility, original-game output proof, or File IO expansion;
- image decode, texture import, audio decode, mesh decode, shader compile,
  material graph, asset database, asset metadata import, editor assets, or
  decoded asset object lifetime;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  RHI objects, RHI resource destruction, or hardware-only proof;
- RenderCore pass scheduling, render graph, frame graph, renderer scheduling,
  command-list parallelism, material binding, scene traversal, UI, World,
  Script, gameplay, reports, screenshots, logs, sleeps, manual visual proof, or
  Game Adapter behavior;
- asynchronous streaming policy, background IO, dependency loading policy,
  cache compression, cache file persistence, or virtual file mounting.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package owns metadata and deterministic entry/load-plan lookup;
- File owns path normalization, mounted reads, and async read completions;
- Streaming owns bounded staging and upload bridges that move source bytes
  toward Resource upload completion values;
- Resource owns identity, handle lifetime, load state, reference counts,
  dependency vocabulary, residency policy, and this proposed opaque cache
  payload ownership;
- asset import/decode later owns interpreting cached bytes as textures, audio,
  meshes, shaders, or materials;
- RHI owns backend-neutral GPU resource primitives and backend-private native
  objects;
- RenderCore and materials later own pass scheduling and bind policy;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep package metadata, file IO,
resource identity, cache storage, asset decode, render scheduling, scene
meaning, and gameplay meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuResource` handle, type, load-state, registry, descriptor,
  slot, dependency, acquire/release, residency, and snapshot value contracts;
- new Resource-owned cache payload value contracts and fixed-capacity byte
  storage records;
- existing `ResourceRegistry` upload completion commit and residency state from
  P2-GATE-021 and P2-GATE-022;
- caller-provided byte spans or fixed byte arrays in tests;
- `Tests/Resource` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuStreaming`, `YuRHI`, `YuPackage`, `YuFile`, `YuRenderCore`, Material,
  World, Script, UI, Game Adapter, parser, decode, report, screenshot, visual
  proof, or original-game evidence dependencies from Resource core;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Resource cache payload headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public Resource contracts may expose value-only descriptors such as:

- Resource cache payload status and operation values;
- Resource handle and expected Resource type validation inputs;
- payload id, payload byte count, payload budget byte count, cache slot index,
  cached byte count, and last status values;
- bounded snapshots for cache payload counters and last operation;
- readback functions that copy opaque bytes into caller-owned output buffers for
  deterministic tests.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, material graph
  nodes, scene ids, UI ids, report handles, screenshot artifacts,
  visual-proof types, or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  package archive internals, parser objects, cache file paths, or original-game
  package layout details.

Public `YuResource` headers must remain free of Streaming, RHI, Package, File,
D3D11, DXGI, Platform, RenderCore, material graph, scene, UI, World, Script, and
Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Resource-owned value contracts for cache payload status, operation,
   request, record, and snapshot.
2. Add bounded ResourceRegistry storage that copies caller-provided opaque bytes
   into fixed-capacity Resource-owned cache payload slots only after Resource
   handle, generation, type, upload state, residency state, payload id, payload
   size, and budget validation succeed.
3. Reject unloaded, failed, non-resident, invalid, stale, type-mismatched,
   duplicate payload id, empty payload, capacity-overflow, and budget-overflow
   requests without mutation.
4. Add readback into caller-owned output buffers so deterministic tests can
   prove exact byte ownership without reports, screenshots, logs, File IO, or
   decode behavior.
5. Add release behavior that clears only Resource-owned cache payload records and
   counters; it must not change load state, residency state, Package state,
   Streaming queues, RHI handles, RenderCore state, or upper-engine bindings.
6. Keep Resource core free of Streaming, RHI, Package, File, RenderCore,
   material graph, render graph, scene, UI, World, Script, and Game Adapter
   dependencies.
7. Add deterministic fast tests for payload store, readback, release, not
   resident rejection, failed-load rejection, stale handle rejection, type
   mismatch, duplicate payload id, capacity overflow, budget overflow, output
   buffer too small, and no mutation on failed validation.
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
- discovery counts for `Resource`, `Streaming`, `Upload`, `RHI`, `RenderCore`,
  `Material`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no Resource
  cache payload tests;
- public-header scan for native/backend/RenderCore/World/Game leakage;
- production dependency scan proving `YuResource` does not gain Streaming, RHI,
  Package, File, RenderCore, material graph, render graph, frame graph, scene,
  UI, World, Script, Game Adapter, original-game parser, or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic byte-for-byte assertions in fast tests over caller-provided
  opaque payloads;
- ResourceRegistry cache payload state and counter assertions through
  Resource-owned public APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource cache payload state, residency state, load commit state, Streaming
  state, Package state, or RHI state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of Resource cache payload proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- File reads, package parsing, asset decode, texture/audio/mesh import,
  RenderCore draw, material graph, render graph, frame graph, scene streaming,
  UI, World, Script, or Game Adapter behavior as evidence for this gate.

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

This evidence keeps cache/decode inside Resource value/status records. It does
not complete Resource residency/upload, RuntimeAsset bridge or CMake
cross-proof, RenderScene/RHI, World/editor/importer, old-package compatibility,
real codec/parser behavior, unrelated animation mapping, Package/Resource
public API expansion, adjacent/full Resource rows, or broad/full CTest.

## Non Goals

- No package parser or original package reader.
- No File IO or mounted archive read behavior.
- No image, audio, mesh, shader, or material decoder.
- No RHI resource destruction or GPU memory ownership.
- No RenderCore pass scheduling.
- No render graph or frame graph.
- No material graph.
- No scene traversal or World streaming.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in Resource-owned opaque cache
   payload value contracts and fixed-capacity byte storage without parser,
   decode, RHI, RenderCore, scene, or upper-engine ownership;
2. implementability review confirms existing ResourceRegistry load-state,
   residency, budget, slot, and snapshot contracts can support the first slice
   locally without adding forbidden dependencies to Resource core;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
