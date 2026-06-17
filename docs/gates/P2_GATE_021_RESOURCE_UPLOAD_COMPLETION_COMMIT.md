# P2-GATE-021: Resource Upload Completion Commit

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-015, P2-GATE-016, P1-GATE-006
Related decisions: ADR-0013
Source baseline: `06184b9`
Proposal commit: `19c4d97`
Landed commit: `475c371`

## Layer

L4-L5 lower-engine Resource/Streaming completion commit bridge over landed
package/resource staging and Resource upload completion values.

This gate proposes the first bounded commit point between completed upload
records and Resource load state. It can consume caller-owned
`ResourceUploadCompletion` values, validate the destination Resource handle and
type, and record deterministic Resource-ready or Resource-failed terminal state
through Resource-owned value contracts. It is not a cache owner, package parser,
asset decoder, render graph, frame graph, RenderCore scheduler, material graph,
scene streaming, UI, World, Script, report, screenshot, manual proof, or Game
Adapter gate.

```text
PackageResourceStagingCompletion values
-> ResourceUploadCompletion values
-> Resource-owned load commit values
-> bounded Resource upload completion commit queue
-> later cache policy, decode pipeline, render graph, scene streaming
```

## Current Reality

P2-GATE-015 landed a package/resource staging queue at `6e29663`. P2-GATE-016
landed a Resource upload queue at `55af599`. The lower-engine resource path can
stage package bytes and upload them through public `YuRHI` value APIs, but
upload completions are still only Streaming-side records. `YuResource` still has
no load-ready or load-failed terminal state, no upload completion commit point,
no cache ownership, no decode/import pipeline, no render graph, no scene
streaming, and no Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `793`;
- `ctest --preset windows-fast-gate -N -L Resource`: `45`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `27`;
- `ctest --preset windows-fast-gate -N -L Upload`: `27`;
- `ctest --preset windows-fast-gate -N -L RHI`: `127`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `40`;
- `ctest --preset windows-fast-gate -N -L Material`: `26`;
- `ctest --preset windows-fast-gate -N -L Fast`: `793`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `67`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `197`;
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

- a narrow Resource-owned load commit vocabulary for terminal upload result
  state such as unloaded, uploaded, and failed without cache ownership;
- ResourceRegistry validation and mutation for a caller-provided load commit
  value, including stale handle, type mismatch, duplicate commit, and invalid
  transition handling;
- a bounded Streaming-side commit queue that consumes `ResourceUploadCompletion`
  values and maps upload success or failure into Resource-owned load commit
  requests;
- value-only descriptors for commit id, upload id, staging request id, resource
  handle, expected resource type, upload byte count, upload status, Resource
  status, and RHI status;
- bounded snapshots for queued, committed, failed, rejected, duplicate, and
  capacity-rejected commit records;
- deterministic tests under default `windows-fast-gate` with `Resource`,
  `Streaming`, `Upload`, `RHI`, `Fast`, `PerformanceSmoke`, and
  `EvidenceOracle` labels;
- proof that failed validation does not mutate Resource slot state, upload
  completion records, RHI output handles, Package state, or staging state.

## Does Not Own

This gate does not own:

- Resource cache ownership, eviction policy, residency policy, lifetime pinning,
  background streaming policy, dependency loading, or acquire/release policy
  redesign;
- package file parsing, original package readers, compression, archive indexing,
  pack/rpack compatibility, original-game output proof, or File IO expansion;
- image decode, texture import, audio decode, mesh decode, shader compile,
  material graph, asset database, asset metadata import, or editor assets;
- direct D3D11, DXGI, Win32, COM, Platform native handles, backend-specific
  upload objects, hardware-only proof, or public native/backend leakage;
- RenderCore pass scheduling, render graph, frame graph, renderer scheduling,
  command-list parallelism, material binding, scene traversal, UI, World,
  Script, gameplay, reports, screenshots, logs, sleeps, manual visual proof, or
  Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package owns metadata and deterministic entry/load-plan lookup;
- File owns path normalization, mounted reads, and async read completions;
- Resource owns identity, handle lifetime, dependency vocabulary, acquire/release
  vocabulary, and terminal load-state records;
- Streaming owns the bounded staging and upload bridges that translate package
  bytes into Resource upload completion values;
- this proposed bridge owns only the bounded commit from upload completion
  values into Resource-owned terminal state;
- RHI owns backend-neutral GPU resource primitives and backend-private native
  objects;
- RenderCore and materials later own pass scheduling and bind policy;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep package metadata, file IO,
resource identity, upload completion, cache ownership, render scheduling, scene
meaning, and gameplay meaning separate.

## Dependencies

Allowed dependencies:

- existing public `YuResource` handle, type, status, registry, descriptor, slot,
  and snapshot value contracts;
- new Resource-owned terminal load-state value contracts that do not depend on
  Streaming, RHI, Package, File, RenderCore, World, Script, UI, or Game Adapter;
- existing `YuStreaming` `ResourceUploadCompletion` value contracts;
- existing public `YuRHI` status and handle values already present inside
  `ResourceUploadCompletion`;
- `Tests/Resource`, `Tests/Streaming`, or equivalent focused tests plus root
  CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- Package parser implementations, original package readers, File IO expansion,
  asset decode/import code, shader compiler/source tooling, RenderCore,
  material graph, render graph, frame graph, scene, UI, World, Script, Game
  Adapter, reports, screenshots, visual proof tooling, manual proof, or
  original-game evidence;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Package, Resource, Streaming, or commit bridge headers;
- adding Streaming, RHI, File, RenderCore, material, scene, UI, World, Script,
  Game Adapter, parser, or decode dependencies to `YuResource` core;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  or original-game output as evidence.

## Public Contract Boundary

Public Resource contracts may expose value-only descriptors such as:

- Resource load state and load commit status values;
- `ResourceHandle` and expected `ResourceTypeId` values for validation;
- commit id, upload id, staging request id, upload byte count, and terminal load
  result values that are independent from Streaming and RHI;
- fixed-capacity Resource registry counters and snapshots for load commit
  outcomes.

Public Streaming commit bridge contracts may expose value-only descriptors such
as:

- `ResourceUploadCompletion` input values;
- caller-owned `ResourceRegistry` pointers or references where existing local
  patterns require mutation;
- fixed-capacity commit queue and completion snapshots.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, material graph
  nodes, scene ids, UI ids, report handles, screenshot artifacts,
  visual-proof types, or Game Adapter types;
- decoded image/audio/mesh payload formats, shader source/compiler handles,
  package archive internals, cache policy objects, or original-game package
  layout details.

Public `YuResource` headers must remain free of Streaming, RHI, Package, File,
D3D11, DXGI, Platform, RenderCore, material graph, scene, UI, World, Script, and
Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Resource-owned value contracts and bounded mutation for terminal
   upload commit state.
2. Add a narrow Streaming-side queue that consumes existing
   `ResourceUploadCompletion` records and commits Resource-ready or
   Resource-failed state through Resource-owned APIs.
3. Validate handle, generation, expected type, duplicate commit id, upload
   status, and terminal-state transition without changing acquire/release or
   dependency policy.
4. Keep Resource core free of Streaming, RHI, Package, File, RenderCore,
   material graph, render graph, scene, UI, World, Script, and Game Adapter
   dependencies.
5. Add deterministic fast tests for upload success commit, failed upload commit,
   invalid Resource handle, type mismatch, duplicate commit id, invalid
   transition, queue overflow, completion overflow where applicable, snapshot
   counters, and no mutation on failed validation.
6. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission is
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
  upload completion commit tests;
- public-header scan for native/backend/RenderCore/World/Game leakage;
- production dependency scan proving `YuResource` does not gain Streaming, RHI,
  Package, File, RenderCore, material graph, render graph, frame graph, scene,
  UI, World, Script, Game Adapter, original-game parser, or decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- ResourceRegistry terminal load-state assertions through Resource-owned public
  APIs;
- Streaming commit queue assertions through existing `ResourceUploadCompletion`
  values;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource state, upload queue state, staging state, or RHI output handles.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of Resource upload completion commit proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- RenderCore draw, material graph, render graph, frame graph, scene streaming,
  UI, World, Script, or Game Adapter behavior as evidence for this gate.

## Non Goals

- No package parser or original package reader.
- No Resource cache ownership or eviction policy.
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

1. boundary review confirms the scope stays in a narrow Resource/Streaming upload
   completion commit bridge without cache, decode, RenderCore, scene, or upper
   engine ownership;
2. implementability review confirms existing ResourceRegistry and
   ResourceUploadCompletion contracts can support the first slice locally
   without adding forbidden dependencies to Resource core;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware-only evidence, reports, screenshots,
   logs, sleeps, manual proof, original-game package outputs, or silent skip.
