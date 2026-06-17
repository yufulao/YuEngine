# P2-GATE-031: Resource Decoded Payload Ownership

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Combined lower-engine review
Depends on: P2-GATE-026, P2-GATE-024, P2-GATE-023, P2-GATE-022, P2-GATE-021, P2-GATE-004
Related decisions: ADR-0011
Source baseline: `c5d5125`
Proposal commit: `97afdb0`
Approval evidence: ENG-153CR combined proposal review PASS.

## Layer

L4-L5 lower-engine Resource decoded-payload ownership boundary over the landed
Resource cache payload, decode plan, and decode-result metadata contracts.

This gate proposes the first bounded Resource decoded-payload slice. The first
slice may accept caller-provided decoded bytes only after a matching landed
decode-result record exists, copy those bytes into Resource-owned fixed-capacity
storage, expose query/readback/release snapshots, and prove failure paths do not
mutate Resource load, residency, cache-payload, or decode-plan state. It is not
a codec gate, decoded object lifetime gate, RHI upload gate, Audio lifecycle
gate, RenderCore gate, Streaming gate, Package/File parser gate, scene/UI/World
gate, Script gate, or Game Adapter gate.

```text
landed cache payload bytes
-> landed decode plan metadata
-> landed import-ready decode-result metadata
-> Resource-owned decoded payload byte storage
-> later import/upload/audio/render ownership gates
```

## Current Reality

P2-GATE-023 landed Resource cache payload ownership at `aca6170`. It stores
opaque cached bytes in Resource-owned fixed-capacity storage.

P2-GATE-024 landed Resource asset decode-plan records at `a6fbabf`. It validates
decode-plan metadata over already cached bytes, but intentionally does not own
decoded output bytes.

P2-GATE-026 landed Resource decode-result import-ready records at `5d28e38`.
It records import-ready metadata such as asset class, result class, decode plan
id, decode result id, and decoded byte count, but it explicitly does not own
decoded byte storage.

P2-GATE-030 landed at `55f618f`, and the queue was marked landed at `c5d5125`.
The accepted post-landing evidence for the lower-engine baseline is:

- full `windows-fast-gate` PASS `914/914`;
- `Resource` `119`;
- `Streaming` `36`;
- `Upload` `43`;
- `RHI` `161`;
- `RenderCore` `67`;
- `Material` `53`;
- `Audio` `53`;
- `AudioResource` `8`;
- `Fast` `914`;
- `PerformanceSmoke` `112`;
- `EvidenceOracle` `318`;
- default `HardwareSmoke` `0`;
- `windows-hardware-smoke` `7` with no Resource decoded-payload entries.

The missing lower-engine boundary is Resource-owned decoded byte payload storage
that is linked to an existing decode-result record. Today Resource can prove
cache payload bytes, decode-plan metadata, and decode-result metadata, but there
is no reviewed Resource unit that stores decoded bytes after metadata
validation, reads them back, releases them, and clears dependent decoded payloads
when earlier Resource-owned records are released.

## Approval Evidence

Approved after ENG-153CR combined proposal review PASS.

Review evidence:

- proposal commit `97afdb039036e5c4778b1e70bf269cd27d5fd178` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 97afdb0^..97afdb0` passed;
- review work was read-only and made no source, doc, implementation, approval,
  commit, or push changes;
- combined review found no `NEEDS_ARCHITECTURE`, no
  `NEEDS_IMPLEMENTABILITY`, no `NEEDS_TEST_POLICY`, and no
  `NEEDS_SCOPE_SPLIT` blocker;
- boundary review confirmed the proposal stays in Resource-owned decoded byte
  payload storage over landed cache payload, decode-plan, and decode-result
  metadata;
- implementability review confirmed existing `ResourceRegistry` cache payload,
  decode-plan, decode-result, fixed-array, budget, validation, and dependent
  clear patterns are sufficient for the first slice;
- test and preset review confirmed deterministic focused
  `Resource_DecodedPayload` evidence, `CMakePresets` no-drift, label
  discovery, and `windows-hardware-smoke` isolation are sufficient;
- public include and production dependency scans found no forbidden Resource
  dependencies on Streaming, Package, File, RHI, RenderCore, Material, Audio,
  AudioResource, scene, UI, World, Script, Game Adapter, or backend-native
  surfaces;
- proof-shape review confirmed screenshots, reports, generated artifacts,
  logs, sleeps, manual proof, audible proof, hardware-only proof, and
  original-game output remain rejected evidence;
- proposal discovery matched the reviewed baseline: default fast gate `914`,
  `Resource` `119`, `Streaming` `36`, `Upload` `43`, `RHI` `161`,
  `RenderCore` `67`, `Material` `53`, `Audio` `53`, `AudioResource` `8`,
  `Fast` `914`, `PerformanceSmoke` `112`, `EvidenceOracle` `318`, default
  `HardwareSmoke` `0`, and `windows-hardware-smoke` `7` with no Resource
  decoded-payload entries.

## Owns

This gate owns the proposal for:

- Resource decoded-payload status, operation, request, record, budget, and
  snapshot value contracts;
- fixed-capacity Resource-owned decoded byte storage keyed by resource handle,
  expected type, payload id, decode plan id, and decode result id;
- validation for invalid resource handle, stale generation, type mismatch,
  missing cache payload, missing decode plan, missing decode result, asset class
  mismatch, result class mismatch, decoded byte count mismatch, empty decoded
  payload, null input bytes, null output storage, output buffer too small,
  duplicate decoded payload id, decoded-payload record capacity overflow, and
  decoded-payload byte budget overflow;
- copying caller-provided decoded bytes into Resource-owned storage only after
  all validation succeeds;
- query, readback, and release behavior for Resource decoded-payload records and
  bytes only;
- dependent cleanup when the owning decode-result, decode-plan, or cache-payload
  record is released;
- snapshots for accepted, rejected, duplicate, capacity, budget, query, read,
  release, and dependent-clear outcomes;
- proof that rejected decoded-payload operations do not mutate Resource load,
  residency, cache payload, decode plan, decode result, Streaming, Upload, RHI,
  RenderCore, Audio, scene, or upper-engine state;
- deterministic tests under default `windows-fast-gate` with `Resource`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- real image, mesh, material, audio, script, or package codec execution;
- decoded object lifetime, texture objects, audio packet lifecycle, mesh buffers,
  material graph data, shader source tooling, or shader compiler behavior;
- RHI upload, GPU resource lifetime, Resource upload queue mutation, Streaming
  queue ownership, File IO expansion, Package parser behavior, or cache-fill
  policy;
- RenderCore scheduling, frame graph execution, command-list parallelism,
  transient resource aliasing, material binding, scene traversal, UI, World,
  Script, gameplay, reports, screenshots, logs, sleeps, manual proof,
  hardware-only proof, original-game evidence, or Game Adapter behavior;
- direct Windows SDK, D3D11, DXGI, COM, Platform native handles, backend-native
  objects, public native/backend leakage, or hardware-smoke admission.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package/File/Streaming later own where bytes come from and how package files
  are read;
- Resource cache payload owns opaque cached byte storage;
- Resource decode plan owns metadata describing how cached bytes should be
  decoded;
- Resource decode result owns import-ready metadata for one decoded output;
- this proposed decoded-payload boundary owns only Resource-controlled decoded
  bytes copied from caller-owned storage after decode-result validation;
- Audio, RHI, RenderCore, material, mesh, scene, and World systems later own
  importing those decoded bytes into their own lifecycle records.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep byte ownership, decode
metadata, import metadata, upload, audio packet lifecycle, renderer scheduling,
scene meaning, and gameplay meaning separate.

## Dependencies

Allowed dependencies:

- landed public `YuResource` handle, load commit, residency, cache payload,
  decode plan, and decode-result value contracts;
- new Resource-owned decoded-payload value contracts and fixed-capacity records;
- `Tests/Resource` focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuStreaming`, `YuPackage`, `YuFile`, `YuRHI`, `YuRenderCore`, `YuMaterial`,
  `YuAudio`, `YuAudioResource`, World, Script, UI, Game Adapter, parser, codec,
  report, screenshot, visual proof, or original-game evidence dependencies from
  Resource production decoded-payload code;
- direct Windows SDK, D3D11, DXGI, COM, Platform, or backend-native types in
  public Resource decoded-payload headers;
- generated reports, screenshots, logs, sleep timing, manual visual inspection,
  hardware-only proof, audible proof, or original-game output as evidence.

## Public Contract Boundary

Public Resource decoded-payload contracts may expose value-only descriptors such
as:

- decoded-payload status and operation values;
- resource handle, expected type, payload id, decode plan id, decode result id,
  asset class, result class, decoded payload id, decoded byte count, record slot,
  and byte offset;
- caller-owned decoded byte input pointers and sizes;
- caller-owned readback output buffers and output byte counts;
- bounded snapshots for decoded-payload counters and last operation;
- query functions that copy decoded-payload records into caller-owned output
  records.

Public contracts must not expose:

- native file handles, Windows handles, D3D11/DXGI/COM types, Platform native
  surfaces, backend-private RHI objects, RenderCore pass ids, Audio voice or
  packet lifecycle internals, Streaming queue records, Package entries, material
  graph nodes, scene ids, UI ids, report handles, screenshot artifacts,
  visual-proof types, or Game Adapter types;
- codec plugin state, decoded image/audio/mesh object layouts, upload handles,
  renderer scheduler state, command-list worker state, original-game package
  structures, or original-game renderer layout details.

Public `YuResource` decoded-payload headers must remain free of Streaming,
Package, File, RHI, RenderCore, Material, Audio, AudioResource, D3D11, DXGI,
Platform, scene, UI, World, Script, and Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts for Resource decoded-payload status, operation,
   request, record, budget, and snapshot values.
2. Add bounded ResourceRegistry storage for decoded-payload records and decoded
   payload bytes.
3. Add a decoded-payload byte budget with explicit invalid and exceeded status
   results.
4. Validate resource handle, expected type, residency/load state, cache payload,
   decode plan, decode result, asset class, result class, decoded byte count,
   input storage, duplicate id, capacity, and budget before mutation.
5. Copy caller-provided decoded bytes into Resource-owned storage only after
   validation succeeds.
6. Add query/readback/release behavior that returns or clears only
   Resource-owned decoded-payload state.
7. Clear dependent decoded-payload records when their decode-result,
   decode-plan, or cache-payload record is released.
8. Keep public and production code free of codec, File, Package, Streaming, RHI,
   RenderCore, Material, Audio, AudioResource, scene, UI, World, Script, Game
   Adapter, native/backend, report, screenshot, log, sleep, and original-game
   evidence dependencies.
9. Add deterministic fast tests for success, query, readback, release, missing
   decode result, stale handle, type mismatch, asset/result class mismatch,
   decoded byte count mismatch, empty payload, null input, null output,
   output buffer too small, duplicate id, capacity overflow, budget overflow,
   dependent clear, and no mutation on failed validation.
10. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission
    is expected for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, real package files, original-game packages, screenshots, reports,
logs, sleeps, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `Resource_DecodedPayload` or equivalent tests proving the new
  boundary;
- discovery counts for `Resource`, `Streaming`, `Upload`, `RHI`, `RenderCore`,
  `Material`, `Audio`, `AudioResource`, `Fast`, `PerformanceSmoke`,
  `EvidenceOracle`, and `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no Resource
  decoded-payload tests;
- public-header scan for native/backend/Streaming/Package/File/RHI/RenderCore/
  Material/Audio/World/Game leakage;
- production dependency scan proving `YuResource` does not gain Streaming,
  Package, File, RHI, RenderCore, Material, Audio, AudioResource, codec,
  scene, UI, World, Script, Game Adapter, original-game, or backend-native
  dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, original-game outputs, audible proof, hardware-only
  proof, and silent skip.

Accepted proof:

- deterministic decoded-payload validation assertions in fast tests;
- Resource decoded-payload record, query, readback, release, snapshot, and
  dependent-clear assertions through public Resource value APIs;
- explicit capacity and validation counters proving failed paths do not mutate
  Resource load state, residency state, cache payloads, decode plans,
  decode-result metadata, Streaming state, Upload state, RHI state, RenderCore
  state, Audio state, or upper-layer state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, audible demos, original-game packages, or
  original-game output;
- silent skip of Resource decoded-payload proof;
- direct D3D11/Win32/native proof as the only acceptance path;
- real codec execution, package parsing, File IO expansion, streaming audio,
  RHI upload, RenderCore scheduling, material graph, frame graph, scene
  traversal, UI, World, Script, or Game Adapter behavior as evidence for this
  gate.

## Non Goals

- No real codec or decoded object format implementation.
- No File, Package, Streaming, or cache-fill expansion.
- No RHI upload or GPU resource lifetime.
- No Audio packet lifecycle or audible playback proof.
- No RenderCore, material graph, renderer scheduling, frame graph, scene
  traversal, World, UI, Script, gameplay, report, screenshot, log, sleep,
  manual proof, hardware-only proof, or Game Adapter behavior.

## Required Review Before Approval

Before `APPROVED_FOR_FIRST_SLICE`, reviewers should independently confirm:

- the proposed boundary is narrower than a codec/import pipeline and only adds
  Resource-owned decoded byte storage over existing decode-result metadata;
- the first slice can be implemented with existing Resource public value
  contracts plus new Resource decoded-payload values;
- no Streaming, Package, File, RHI, RenderCore, Material, Audio, AudioResource,
  scene, UI, World, Script, Game Adapter, backend-native, report, screenshot,
  log, sleep, manual proof, hardware-only proof, audible proof, or original-game
  evidence dependency is required;
- deterministic fast tests and label evidence are sufficient for the first
  slice;
- CMake presets do not need to change unless labels for new tests require root
  CMake target updates.
