# P2-GATE-029: Audio Resource PCM Packet Import Bridge

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Split lower-engine review
Depends on: P2-GATE-026, P2-GATE-028, P2-GATE-024, P2-GATE-004, ADR-0012, ADR-0013
Related decisions: ADR-0012, ADR-0013
Source baseline: `6ec53b3`
Proposal commit: `5e80cd7`
Landed commit: `0a22dac`
Approval evidence: ENG-149A, ENG-149B, ENG-149C, ENG-149D, ENG-149E, and ENG-149F proposal review PASS.
Discovery evidence: ENG-149R candidate scan PASS.

## Layer

L4-L5 lower-engine bridge over landed Resource decode-result metadata and
landed Audio PCM sample packet value contracts.

This gate proposes the first bounded bridge-owned mapping from Resource
decode-result metadata to Audio PCM sample packet request descriptors. The
first slice may validate an active Resource decode-result record for audio,
accept caller-provided PCM packet descriptor fields, derive frame and sample
counts from decoded byte metadata, record deterministic bridge metadata, expose
query and release snapshots, and prove counter behavior. It must not store
decoded bytes, run a codec, read files, parse packages, create streaming audio,
move Resource ownership into Audio core, or move Audio packet lifecycle into
Resource core.

```text
landed Resource decode-result metadata
-> bridge-owned Audio import request
-> Audio PCM sample packet request descriptor
-> bounded bridge metadata and counters
-> later decoded-byte handoff or streaming policy
```

## Current Reality

P2-GATE-026 landed Resource-owned import-ready decode-result metadata at
`5d28e38`. Resource can record active decode-result metadata with asset class,
result class, decoded byte count, decode plan id, payload id, and Resource
identity without storing decoded bytes.

P2-GATE-028 landed Audio-owned PCM sample packet value contracts at `534d3ef`.
Audio can validate caller-provided fixed S16 stereo packet descriptors, record
bounded packet-slot metadata, query/release packet records, and expose
deterministic counters without Resource, File, Package, codec, or streaming
ownership.

The missing boundary is the bridge between these two lower-engine contracts:
there is no reviewed place to prove that an active Resource audio decode-result
metadata record can produce an Audio PCM packet request descriptor while keeping
both cores ownership-clean. This gate proposes that bridge as its own boundary.
It does not propose real audio decode, decoded byte storage, asset loading,
package parsing, streaming audio, mixer graph expansion, audio scene routing,
business sound ids, UI, World, Script, or Game Adapter behavior.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `893`;
- `ctest --preset windows-fast-gate -N -L Audio`: `45`;
- `ctest --preset windows-fast-gate -N -L Resource`: `111`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `36`;
- `ctest --preset windows-fast-gate -N -L Upload`: `43`;
- `ctest --preset windows-fast-gate -N -L RHI`: `148`;
- `ctest --preset windows-fast-gate -N -L RenderCore`: `54`;
- `ctest --preset windows-fast-gate -N -L Material`: `40`;
- `ctest --preset windows-fast-gate -N -L Fast`: `893`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `104`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `297`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Audio`: `1`;
- `ctest --preset windows-hardware-smoke -N -L Resource`: `0`.

## Approval Evidence

Approved after ENG-149A, ENG-149B, ENG-149C, ENG-149D, ENG-149E, and
ENG-149F proposal reviews PASS.

Review evidence:

- proposal commit `5e80cd7982bbf31a5634af48514faac4a6e9f027` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 5e80cd7^..5e80cd7` passed;
- review work was read-only and made no source, doc, commit, push, approval, or
  implementation changes;
- ENG-149R recommended an Audio Resource PCM Packet Import Bridge as the next
  lower-engine candidate after P2-GATE-028 landing;
- the key boundary is bridge-owned cross-module mapping, not direct
  `YuResource` ownership in `YuAudio` core and not Audio packet lifecycle in
  Resource core;
- boundary and quality review confirmed the proposal stays in bridge-owned
  metadata mapping and has no boundary blocker;
- implementability review confirmed current Resource decode-result and Audio
  PCM packet public contracts are sufficient for an independent bridge-owned
  first slice;
- test-policy review confirmed deterministic focused bridge tests, label
  discovery, CMakePresets no-drift, hardware-smoke isolation, and proof-shape
  requirements are sufficient;
- candidate-consistency review confirmed the proposal did not widen ENG-149R
  into decode, storage, streaming, or gameplay scope;
- public dependency and leak review confirmed no public dependency,
  native/backend, Audio core, or Resource core leak blocker;
- implementation preflight review confirmed the first slice can be implemented
  as bridge-owned value contracts and bounded metadata without touching codec,
  byte storage, streaming, or upper-engine systems;
- proposal discovery counts matched the reviewed baseline: default fast gate
  `893`, `Audio` `45`, `Resource` `111`, `Streaming` `36`, `Upload` `43`,
  `RHI` `148`, `RenderCore` `54`, `Material` `40`, `Fast` `893`,
  `PerformanceSmoke` `104`, `EvidenceOracle` `297`, default `HardwareSmoke`
  `0`, and `windows-hardware-smoke` `7` with `Audio` `1` and `Resource` `0`.

## Owns

This gate owns the proposal for:

- bridge-owned PCM import status, operation, request, record, and snapshot
  value contracts;
- fixed-capacity bridge records linking one active Resource decode-result id to
  one Audio PCM packet id;
- validation for Resource handle, generation, expected Resource type,
  decode-result id, decode-result active state, audio asset class, audio result
  class, decoded byte count, packet id, sample format, sample rate, channel
  count, frame count, interleaved sample count, byte count, duplicate bridge
  id, duplicate packet id, stale Resource metadata, bridge capacity, and
  release state;
- deterministic mapping from decoded byte count to Audio PCM packet request
  byte count, frame count, and interleaved sample count for fixed S16 stereo
  metadata;
- query and release behavior for bridge-owned mapping metadata;
- deterministic snapshots for accepted, rejected, capacity rejected,
  duplicate, query, release, and stale-record outcomes;
- proof that rejected bridge requests do not mutate Resource decode-result
  state, Audio packet state, File, Package, Streaming, RHI, RenderCore, UI,
  World, Script, or Game Adapter state;
- deterministic tests under default `windows-fast-gate` with `Audio`,
  `Resource`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- decoded audio byte storage, byte copying, decoded object lifetime, audio
  file loading, package lookup, mounted file reads, archive parsing, original
  package readers, compression, codec parsing, real decode, resample,
  streaming, async decode, or Resource-backed asset lifetime policy;
- BGM/SE business ids, original sound names, audio scene routing, UI sound
  behavior, Script audio services, gameplay audio, World behavior, reports,
  screenshots, logs, sleeps, manual listening proof, audible proof, or Game
  Adapter behavior;
- RHI upload, RenderCore scheduling, render graph, material graph, scene
  traversal, GPU resource lifetime, backend-native public headers, or hardware
  device proof;
- changing Resource core ownership, changing Audio core ownership, or making
  default `windows-fast-gate` depend on an audio endpoint, package file, or real
  decoded asset.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Resource owns asset identity, load state, cache payload ownership,
  decode-plan metadata, and import-ready decode-result metadata;
- Audio owns sample format vocabulary, packet metadata, explicit packet
  lifecycle, source/voice mix contracts, callback submission values, and
  deterministic packet counters;
- this bridge owns the cross-module mapping from Resource decode-result
  metadata into Audio PCM packet request descriptors;
- later decoded-byte handoff or streaming policy may connect real decoded audio
  bytes after a separate gate reviews byte ownership and lifetime;
- UI, Script, World, and Game Adapter later own business sound routing and
  gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep Resource import metadata,
Audio packet lifecycle, decoded byte ownership, streaming, and gameplay audio
separate.

## Dependencies

Allowed dependencies:

- existing public `YuResource` handle, type, decode-plan, decode-result
  request, decode-result record, decode-result class, and snapshot value
  contracts;
- existing public `YuAudio` sample format, PCM sample packet request, record,
  handle, operation, and snapshot value contracts;
- new bridge-owned PCM import status, operation, request, record, and snapshot
  value contracts;
- private bridge implementation storage for bounded mapping metadata;
- `Tests/AudioResource`, `Tests/Audio`, `Tests/Resource`, or equivalent
  deterministic tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- adding `YuResource`, `YuStreaming`, `YuPackage`, `YuFile`, `YuRHI`,
  `YuRenderCore`, Material, World, Script, UI, Game Adapter, parser, real
  decoder, report, screenshot, visual proof, or original-game evidence
  dependencies to production `YuAudio` PCM packet core;
- adding `YuAudio`, mixer, callback device, source, voice, BGM/SE, UI, Script,
  World, Game Adapter, report, screenshot, audible proof, or hardware endpoint
  dependencies to production `YuResource` core;
- direct Windows SDK, XAudio2, WASAPI, D3D11, DXGI, COM, Platform, backend, or
  hardware endpoint types in public bridge headers;
- generated reports, screenshots, logs, sleep timing, manual listening,
  audible output, hardware-only proof, or original-game output as evidence.

## Public Contract Boundary

Public bridge contracts may expose value-only descriptors such as:

- bridge PCM import status and operation values;
- Resource handle, expected Resource type, decode result id, packet id, sample
  format, sample rate, channel count, decoded byte count, frame count,
  interleaved sample count, byte count, and bridge slot values;
- bounded snapshots for bridge counters and last operation;
- query functions that return mapping metadata without exposing file paths,
  package entries, decoded bytes, parser state, decoded object pointers, audio
  device handles, callback objects, or backend-private audio objects.

Public bridge contracts must not expose:

- Windows headers, XAudio2, WASAPI, COM, endpoint handles, callback objects,
  thread handles, native buffers, File paths, Package ids, RHI handles,
  RenderCore pass ids, material ids, scene ids, UI ids, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types;
- codec bitstream formats, package archive internals, parser objects, cache
  file paths, decoded asset object pointers, or original-game package layout
  details.

Public `YuAudio` headers must remain free of Resource, Streaming, Package,
File, RHI, RenderCore, material graph, scene, UI, World, Script, and Game
Adapter dependencies.

Public `YuResource` headers must remain free of Audio device, mixer, source,
voice, callback, BGM/SE, UI, World, Script, and Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only bridge-owned value contracts for PCM import status, operation,
   request, record, and snapshot.
2. Add bounded bridge storage for mapping metadata from active Resource
   decode-result records to Audio PCM packet request descriptors.
3. Validate Resource handle, generation, expected Resource type,
   decode-result id, active decode-result record, audio asset class, audio
   result class, decoded byte count, packet id, sample format, sample rate,
   channel count, frame count, sample count, byte count, duplicate bridge id,
   duplicate packet id, stale records, capacity, and release state before
   mutation.
4. Reject missing decode result, stale Resource handle, wrong Resource type,
   inactive decode result, non-audio asset class, non-audio result class, zero
   decoded byte count, invalid sample format, unsupported sample rate,
   unsupported channel count, frame-count mismatch, sample-count mismatch,
   byte-count mismatch, duplicate bridge id, duplicate packet id, capacity
   overflow, stale bridge handle, and released bridge queries without mutating
   Resource decode-result state, Audio packet state, File, Package, Streaming,
   RHI, RenderCore, UI, World, Script, or Game Adapter state.
5. Add query and release behavior that returns or clears only bridge-owned
   mapping metadata.
6. Keep existing ResourceRegistry decode-result behavior and Audio PCM packet
   behavior compatible and covered by regression tests.
7. Keep Audio core free of Resource dependencies and Resource core free of
   Audio lifecycle dependencies.
8. Add deterministic fast tests for successful metadata-to-packet-request
   mapping, query, release, missing decode result, stale Resource handle,
   wrong Resource type, inactive decode result, non-audio classes, zero decoded
   byte count, invalid packet metadata, duplicate bridge id, duplicate packet
   id, capacity overflow, stale bridge handle, no mutation on rejection, and
   snapshot counters.
9. Keep all required proof in default `windows-fast-gate`; no new
   hardware-smoke admission is required for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
audio hardware, audio files, original-game packages, screenshots, reports,
logs, sleeps, audible output, decoded asset bytes, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `AudioResource_PcmPacketImportBridge` or equivalent tests proving the
  new boundary;
- discovery counts for `Audio`, `Resource`, `Streaming`, `Upload`, `RHI`,
  `RenderCore`, `Material`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke`;
- proof that default `windows-fast-gate` still discovers `HardwareSmoke` as
  `0`;
- proof that existing `windows-hardware-smoke` remains isolated and that no new
  bridge proof depends on a real endpoint, package file, or decoded asset;
- public-header scan for native/backend/File/Package/RHI/RenderCore/World
  leakage;
- production dependency scan proving `YuAudio` core does not gain Resource,
  Streaming, Package, File, RHI, RenderCore, material graph, render graph,
  scene, UI, World, Script, Game Adapter, original-game parser, or real decode
  dependencies;
- production dependency scan proving `YuResource` core does not gain Audio
  device, mixer, source, voice, callback, BGM/SE, UI, World, Script, Game
  Adapter, audible proof, or hardware endpoint dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, audible proof, original-game outputs, hardware-only
  proof, and silent skip.

Accepted proof:

- deterministic assertions over public bridge request, record, query, release,
  snapshot, Audio PCM packet request descriptor, and counter values;
- explicit capacity and validation counters proving failed paths do not mutate
  bridge state, Resource decode-result state, Audio packet state, File state,
  Package state, RHI state, RenderCore state, or upper-engine state;
- regression assertions that existing `Resource_DecodeResult` and
  `Audio_PcmSamplePacket` fast tests remain deterministic.

Rejected proof:

- screenshots, reports, generated audio/image artifacts, manual listening,
  audible playback, logs, sleeps, visual demos, original-game packages, or
  original-game output;
- silent skip of bridge proof;
- direct XAudio2/Win32/native proof as the only acceptance path;
- Resource File read, Package parse, codec decode, decoded byte storage,
  streaming, BGM/SE service, UI, World, Script, Game Adapter, RenderCore
  scheduling, RHI upload, or hardware-only behavior as evidence for this gate.

## Non Goals

- No audio file decoder, codec parser, decoded byte storage, byte copying,
  resampler, or streaming buffer.
- No File IO, mounted archive read, Package parsing, or original package
  reader.
- No Resource ownership inside Audio core and no Audio packet lifecycle inside
  Resource core.
- No automatic Audio packet creation that consumes decoded bytes.
- No mixer graph, panning, envelopes, effects, spatialization, audio scene, or
  business sound routing.
- No BGM/SE ids, original sound names, UI sound behavior, Script audio service,
  World behavior, gameplay, report, screenshot, log, sleep, manual listening
  proof, audible proof, or Game Adapter behavior.
- No RHI upload, RenderCore scheduling, material graph, render graph, frame
  graph, or backend-native public exposure.
- No default fast-gate real-device, package-file, decoded-asset, or
  original-game requirement.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after split lower-engine review
confirms the proposal stays in bridge-owned metadata mapping from landed
Resource decode-result records to landed Audio PCM packet request descriptors,
remains implementable with current public value contracts, preserves
deterministic fast-gate proof, and does not introduce decoded bytes, Resource
ownership in Audio core, Audio lifecycle ownership in Resource core, real
codec, streaming, UI, World, Script, Game Adapter, report, screenshot, log,
sleep, manual, audible, hardware-only, or original-game evidence.
