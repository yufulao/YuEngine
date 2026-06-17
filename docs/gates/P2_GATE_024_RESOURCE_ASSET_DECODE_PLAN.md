# P2-GATE-024: Resource Asset Decode Plan

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-023, P2-GATE-022, P2-GATE-021, P1-GATE-006
Related decisions: ADR-0013
Source baseline: `2112056`

## Layer

L4-L5 lower-engine Resource decode-plan boundary over landed Resource identity,
upload completion commit state, residency budget policy, and cache payload
ownership.

This gate proposes the first bounded Resource-owned decode/import planning
contract. It may validate cached opaque payload bytes through a fixed,
Resource-owned test header and record deterministic decode-plan metadata,
counters, and release behavior. It does not read files, parse packages, run
image/audio/mesh codecs, create decoded asset objects, import textures, upload
to RHI, schedule RenderCore work, traverse scenes, drive UI, touch World, run
Script, or define Game Adapter behavior.

```text
Resource-owned opaque cache payload bytes and records
-> Resource-owned decode plan records
-> later real texture/audio/mesh decode, import, render graph, scene streaming
```

## Current Reality

P2-GATE-015 landed a package/resource staging queue at `6e29663`. P2-GATE-016
landed a Resource upload queue at `55af599`. P2-GATE-021 landed Resource upload
completion commit at `475c371`. P2-GATE-022 landed Resource residency budget
policy at `d2f2059`. P2-GATE-023 landed Resource cache payload ownership at
`aca6170`. `YuResource` can now own uploaded/resident Resource state and
opaque cached payload bytes, but it still has no Resource-owned decode plan, no
real asset codec, no decoded object lifetime, no render graph, no scene
streaming, and no Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `832`;
- `ctest --preset windows-fast-gate -N -L Resource`: `84`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `36`;
- `ctest --preset windows-fast-gate -N -L Upload`: `43`;
- `ctest --preset windows-fast-gate -N -L RHI`: `127`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `40`;
- `ctest --preset windows-fast-gate -N -L Material`: `26`;
- `ctest --preset windows-fast-gate -N -L Fast`: `832`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `83`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `236`;
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

- Resource-owned decode-plan status values for success, invalid handle,
  generation mismatch, type mismatch, not uploaded, not resident, missing cache
  payload, invalid payload id, invalid header, unsupported header version,
  duplicate plan id, capacity exceeded, budget exceeded, and invalid argument;
- Resource-owned decode-plan request values containing Resource handle,
  expected Resource type, cache payload id, decode plan id, asset class, source
  byte count, and expected decoded byte count;
- fixed-capacity Resource-owned decode-plan records that reference landed cache
  payload records without copying bytes into a decoded asset object;
- deterministic validation of a fixed Resource test header inside cached bytes
  so tests can prove the boundary without real image/audio/mesh codecs;
- ResourceRegistry APIs that create, query, release, and snapshot decode-plan
  records only after Resource handle, type, upload state, residency state, cache
  payload, header, capacity, and budget validation succeed;
- proof that failed validation does not mutate Resource slot state, cache
  payload records, residency state, load commit state, Package state, Streaming
  queues, RHI state, RenderCore state, or upper-engine bindings;
- deterministic tests under default `windows-fast-gate` with `Resource`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- package file parsing, original package readers, compression, archive indexing,
  pack/rpack compatibility, original-game output proof, or File IO expansion;
- image decode, texture import, audio decode, mesh decode, shader compile,
  material graph, asset database, asset metadata import, editor assets, decoded
  asset object lifetime, or cache file persistence;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific RHI
  objects, RHI resource destruction, RHI upload execution, or hardware-only
  proof;
- RenderCore pass scheduling, render graph, frame graph, renderer scheduling,
  command-list parallelism, material binding, scene traversal, UI, World,
  Script, gameplay, reports, screenshots, logs, sleeps, manual visual proof, or
  Game Adapter behavior;
- asynchronous streaming policy, background IO, dependency loading policy,
  cache compression, virtual file mounting, or real asset format ownership.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package owns metadata and deterministic entry/load-plan lookup;
- File owns path normalization, mounted reads, and async read completions;
- Streaming owns bounded staging and upload bridges that move source bytes
  toward Resource upload completion values;
- Resource owns identity, handle lifetime, load state, reference counts,
  dependency vocabulary, residency policy, opaque cache payload ownership, and
  this proposed decode-plan metadata boundary;
- asset decode/import later owns interpreting real texture, audio, mesh, shader,
  or material formats and producing decoded object data;
- RHI owns backend-neutral GPU resource primitives and backend-private native
  objects;
- RenderCore and materials later own pass scheduling and bind policy;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep package metadata, file IO,
resource identity, cache storage, decode planning, real asset decode, render
scheduling, scene meaning, and gameplay meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuResource` handle, type, load-state, registry, descriptor,
  slot, dependency, acquire/release, residency, cache payload, and snapshot value
  contracts;
- new Resource-owned decode-plan status, request, record, operation, asset class,
  and snapshot value contracts;
- existing ResourceRegistry cache payload storage from P2-GATE-023;
- fixed caller-provided bytes already stored in Resource cache payload records;
- `Tests/Resource` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuStreaming`, `YuRHI`, `YuPackage`, `YuFile`, `YuRenderCore`, Material,
  World, Script, UI, Game Adapter, parser, real decoder, report, screenshot,
  visual proof, or original-game evidence dependencies from Resource core;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Resource decode-plan headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public Resource contracts may expose value-only descriptors such as:

- Resource decode-plan status and operation values;
- Resource handle and expected Resource type validation inputs;
- cache payload id, decode plan id, asset class, source byte count, decoded byte
  count, fixed header version, and cache slot index values;
- bounded snapshots for decode-plan counters and last operation;
- query functions that return decode-plan metadata without exposing decoded
  asset objects or backing byte pointers.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, material graph
  nodes, scene ids, UI ids, report handles, screenshot artifacts,
  visual-proof types, or Game Adapter types;
- real decoded image/audio/mesh payload formats, shader source/compiler handles,
  package archive internals, parser objects, cache file paths, decoded asset
  object pointers, or original-game package layout details.

Public `YuResource` headers must remain free of Streaming, RHI, Package, File,
D3D11, DXGI, Platform, RenderCore, material graph, scene, UI, World, Script, and
Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Resource-owned value contracts for decode-plan status, operation,
   request, record, asset class, and snapshot.
2. Add bounded ResourceRegistry storage for decode-plan records that reference
   existing cache payload records without copying bytes into decoded asset
   objects.
3. Validate Resource handle, generation, type, upload state, residency state,
   cache payload id, fixed header magic/version, source byte count, decoded byte
   count, duplicate plan id, record capacity, and budget before mutation.
4. Reject missing cache payload, invalid header, unsupported version, stale
   handle, type mismatch, not resident, failed load, duplicate plan id, capacity
   overflow, and budget overflow without mutating Resource slot, residency,
   cache payload, Streaming, Package, RHI, RenderCore, or upper-engine state.
5. Add query and release behavior that returns or clears only Resource-owned
   decode-plan metadata.
6. Keep Resource core free of Streaming, RHI, Package, File, RenderCore,
   material graph, render graph, scene, UI, World, Script, and Game Adapter
   dependencies.
7. Add deterministic fast tests for decode-plan success from cached bytes,
   metadata query, release, missing cache payload, invalid header, unsupported
   version, stale handle, type mismatch, not resident, failed load, duplicate
   plan id, capacity overflow, budget overflow, and no mutation on failed
   validation.
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
- focused `Resource_DecodePlan` or equivalent tests proving the new boundary;
- discovery counts for `Resource`, `Streaming`, `Upload`, `RHI`, `RenderCore`,
  `Material`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no Resource
  decode-plan tests;
- public-header scan for native/backend/RenderCore/World/Game leakage;
- production dependency scan proving `YuResource` does not gain Streaming, RHI,
  Package, File, RenderCore, material graph, render graph, frame graph, scene,
  UI, World, Script, Game Adapter, original-game parser, or real decode
  dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic assertions over caller-provided cached bytes with a fixed test
  header;
- ResourceRegistry decode-plan state and counter assertions through
  Resource-owned public APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource decode-plan state, cache payload state, residency state, load commit
  state, Streaming state, Package state, RHI state, or RenderCore state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of Resource decode-plan proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- File reads, package parsing, real image/audio/mesh decode, texture/audio/mesh
  import, RenderCore draw, material graph, render graph, frame graph, scene
  streaming, UI, World, Script, or Game Adapter behavior as evidence for this
  gate.

## Non Goals

- No package parser or original package reader.
- No File IO or mounted archive read behavior.
- No real image, audio, mesh, shader, or material decoder.
- No decoded asset object lifetime or cache-file persistence.
- No RHI upload execution or GPU memory ownership.
- No RenderCore pass scheduling.
- No render graph or frame graph.
- No material graph.
- No scene traversal or World streaming.
- No UI, Script, gameplay, report, screenshot, log, sleep, manual proof, or Game
  Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in Resource-owned decode-plan value
   contracts over existing cached bytes without File, Package, real decoder,
   RHI, RenderCore, scene, or upper-engine ownership;
2. implementability review confirms existing ResourceRegistry load-state,
   residency, cache payload, slot, and snapshot contracts can support the first
   slice locally without adding forbidden dependencies to Resource core;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, real codec proof,
   or silent skip.
