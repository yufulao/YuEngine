# P2-GATE-022: Resource Residency Budget Policy

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-021, P1-GATE-006
Related decisions: ADR-0013
Source baseline: `c650d49`
Proposal commit: `07e94ac`
Landed commit: `d2f2059`

## Layer

L4-L5 lower-engine Resource residency policy over landed Resource identity,
reference counting, dependency edges, and upload completion commit state.

This gate proposes the first bounded Resource-owned residency budget policy. It
can classify uploaded Resource slots as resident, pinned, evictable, or evicted
through Resource-owned value contracts and deterministic counters. It does not
store decoded payloads, own GPU memory, release RHI objects, parse packages,
read files, decode assets, schedule RenderCore work, traverse scenes, drive UI,
touch World, run Script, or define Game Adapter behavior.

```text
ResourceUploadCompletion values
-> Resource-owned load commit state
-> Resource-owned residency state and budget policy
-> later cache storage, decode pipeline, render graph, scene streaming
```

## Current Reality

P2-GATE-015 landed a package/resource staging queue at `6e29663`. P2-GATE-016
landed a Resource upload queue at `55af599`. P2-GATE-021 landed a Resource
upload completion commit bridge at `475c371`. `YuResource` can now record
terminal upload load state and existing acquire/release counts, but it still has
no Resource-owned residency vocabulary, no bounded cache budget, no eviction
candidate decision, no cache payload storage, no decode/import pipeline, no
render graph, no scene streaming, and no Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `809`;
- `ctest --preset windows-fast-gate -N -L Resource`: `61`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `36`;
- `ctest --preset windows-fast-gate -N -L Upload`: `43`;
- `ctest --preset windows-fast-gate -N -L RHI`: `127`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `40`;
- `ctest --preset windows-fast-gate -N -L Material`: `26`;
- `ctest --preset windows-fast-gate -N -L Fast`: `809`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `72`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `213`;
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

- Resource-owned residency state values for unloaded, uploaded, resident,
  pinned, evictable, evicted, and failed Resource slots where a narrower enum is
  acceptable if review finds overlap with existing load state;
- a bounded Resource residency budget descriptor with byte capacity and resident
  byte counters;
- ResourceRegistry mutation APIs that admit only successfully uploaded Resource
  slots into resident state;
- deterministic pin and unpin behavior using Resource-owned handle and type
  validation without depending on World or RenderCore;
- deterministic eviction candidate selection that never chooses an acquired,
  pinned, failed, unloaded, or stale Resource slot;
- fixed-capacity residency records and snapshots for admitted, pinned,
  unpinned, evicted, rejected, budget-rejected, and candidate-miss outcomes;
- proof that failed validation does not mutate Resource slot state, load commit
  state, dependency edges, Package state, Streaming queues, RHI state, or any
  upper-engine binding;
- deterministic tests under default `windows-fast-gate` with `Resource`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- cache payload storage, cache line allocation, memory allocator ownership,
  eviction execution, background streaming, dependency loading, or async policy;
- package file parsing, original package readers, compression, archive indexing,
  pack/rpack compatibility, original-game output proof, or File IO expansion;
- image decode, texture import, audio decode, mesh decode, shader compile,
  material graph, asset database, asset metadata import, or editor assets;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  RHI objects, RHI resource destruction, or hardware-only proof;
- RenderCore pass scheduling, render graph, frame graph, renderer scheduling,
  command-list parallelism, material binding, scene traversal, UI, World,
  Script, gameplay, reports, screenshots, logs, sleeps, manual visual proof, or
  Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Resource owns identity, handle lifetime, load state, reference counts,
  dependencies, and the first residency/budget vocabulary;
- Streaming owns staging and upload bridges that produce Resource upload
  completion and commit values;
- File and Package own source discovery and bytes, not Resource residency;
- RHI owns backend-neutral GPU resource primitives and backend-private native
  objects;
- RenderCore and materials later own pass scheduling and bind policy;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep resource identity, residency
policy, asset decode, cache storage, render scheduling, scene meaning, and
gameplay meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuResource` handle, type, load-state, registry, descriptor,
  slot, dependency, acquire/release, and snapshot value contracts;
- new Resource-owned residency and budget value contracts;
- existing `ResourceRegistry` upload completion commit state from P2-GATE-021;
- `Tests/Resource` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuStreaming`, `YuRHI`, `YuPackage`, `YuFile`, `YuRenderCore`, Material,
  World, Script, UI, Game Adapter, parser, decode, report, screenshot, visual
  proof, or original-game evidence dependencies from Resource core;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Resource residency headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public Resource contracts may expose value-only descriptors such as:

- Resource residency state and residency operation status values;
- budget capacity, resident byte count, pinned byte count, and evictable byte
  count values;
- caller-owned handle/type validation inputs;
- fixed-capacity snapshots for residency counters and last status;
- eviction candidate handles selected by Resource-owned policy.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, material graph
  nodes, scene ids, UI ids, report handles, screenshot artifacts,
  visual-proof types, or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  package archive internals, cache storage pointers, or original-game package
  layout details.

Public `YuResource` headers must remain free of Streaming, RHI, Package, File,
D3D11, DXGI, Platform, RenderCore, material graph, scene, UI, World, Script, and
Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Resource-owned value contracts for residency state, residency
   status, budget descriptor, residency request, and residency snapshot.
2. Add bounded ResourceRegistry mutation that admits uploaded Resource slots into
   resident state and rejects unloaded, failed, invalid, stale, type-mismatched,
   duplicate, or budget-overflow requests without mutation.
3. Add pin and unpin behavior that respects existing acquire/reference state and
   does not require World, RenderCore, Streaming, Package, File, or RHI.
4. Add deterministic eviction candidate selection that excludes acquired,
   pinned, failed, unloaded, stale, and non-resident Resources.
5. Keep Resource core free of Streaming, RHI, Package, File, RenderCore,
   material graph, render graph, scene, UI, World, Script, and Game Adapter
   dependencies.
6. Add deterministic fast tests for resident admission, unloaded rejection,
   failed-load rejection, type mismatch, duplicate admission, budget overflow,
   pin/unpin counters, eviction candidate ordering, no candidate, and no
   mutation on failed validation.
7. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission is
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
  residency tests;
- public-header scan for native/backend/RenderCore/World/Game leakage;
- production dependency scan proving `YuResource` does not gain Streaming, RHI,
  Package, File, RenderCore, material graph, render graph, frame graph, scene,
  UI, World, Script, Game Adapter, original-game parser, or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- ResourceRegistry residency-state and budget-counter assertions through
  Resource-owned public APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource state, load commit state, dependency edges, Streaming state, Package
  state, or RHI state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of Resource residency proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- File reads, package parsing, asset decode, RenderCore draw, material graph,
  render graph, frame graph, scene streaming, UI, World, Script, or Game Adapter
  behavior as evidence for this gate.

## L0-RES-004 Evidence Sync

L0-RES-004 Resource residency/upload chain closure is PASS at
`45f91f6cda02e42f0dce7eae7ff3df6db3616467` for this gate's residency budget
surface. Focused QA task `2917323c-9869-4a1c-a9fb-67a90b513a23` reports
`YuStreamingTests` and `YuResourceTests` build PASS,
`Streaming_ResourceUpload_` discovery/execution `17/17` PASS,
`Streaming_ResourceUploadCommit_` discovery/execution `9/9` PASS,
`Resource_LoadCommit_`/`Resource_Residency_` discovery/execution `18/18` PASS,
combined focused execution `44/44` PASS, and a clean read-only QA workspace.
Readiness task `d88846fd` records existing Resource/Streaming/RHI value/status
records for upload queue, upload commit, Resource load commit, residency
budget/state, pin/unpin/eviction, and stale/invalid handle no-mutation.

This sync keeps residency evidence as Resource-owned budget and state policy.
QA did not build or execute `YuRHITests`, RHI 38-row dependency execution,
adjacent/full Resource, full `^Resource_`, or broad/full CTest. RHI primitive
evidence remains a dependency note, and `L0-RHI-003` is not table-closed by
this gate update.

## Non Goals

- No package parser or original package reader.
- No cache payload storage or memory allocator ownership.
- No eviction execution against File, Streaming, RHI, RenderCore, World, or
  Game Adapter state.
- No full dependency-driven load scheduler.
- No image, audio, mesh, shader, or material decoder.
- No RenderCore pass scheduling.
- No render graph or frame graph.
- No material graph.
- No scene traversal or World streaming.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in Resource-owned residency and
   budget value contracts without cache storage, decode, RenderCore, scene, or
   upper-engine ownership;
2. implementability review confirms existing ResourceRegistry load-state,
   acquire/release, dependency, and snapshot contracts can support the first
   slice locally without adding forbidden dependencies to Resource core;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
