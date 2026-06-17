# P2-GATE-028: Audio PCM Sample Packet

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: Combined lower-engine review
Depends on: P2-GATE-002, P2-GATE-011, P2-GATE-004, ADR-0012
Related decisions: ADR-0012
Source baseline: `a5feded`
Proposal commit: `b9389ee`
Landed commit: `534d3ef`
Approval evidence: ENG-147A, ENG-147B, and ENG-147C proposal review PASS.

## Layer

L3 lower-engine Audio sample packet boundary over landed test backend, mixer
sink, and private callback backend contracts.

This gate proposes the first bounded Audio-owned PCM sample packet value
contract. The first slice may validate caller-provided fixed-format S16 stereo
packet descriptors, record deterministic sample-slot metadata, expose query and
release snapshots, and prove counter behavior without reading files, decoding
codecs, streaming from Resource, or creating BGM/SE, scene, UI, World, Script,
or Game Adapter behavior.

```text
landed Audio test backend and callback contracts
-> caller-provided fixed S16 PCM packet descriptor
-> Audio-owned bounded packet slot metadata
-> deterministic query, release, and counter proof
-> later Resource-backed audio import bridge
```

## Current Reality

P2-GATE-002 landed deterministic `YuAudio` test backend and mixer sink values.
P2-GATE-011 landed private Windows XAudio2 callback backend proof and public
callback value contracts. Audio can register synthetic S16 sources, mix into
caller-owned buffers, and submit fixed S16 callback buffers, but it still has no
standalone PCM packet contract that can later receive decoded audio samples
without pulling Resource, File, Package, codec, UI, World, or Game Adapter
ownership into Audio.

The missing boundary is an Audio-owned value record for a bounded PCM sample
packet: packet id, generation, format, sample rate, channel count, frame count,
sample count, byte count, release state, and deterministic counters. This gate
does not propose real decode output, asset loading, package parsing, Resource
integration, streaming audio, mixer graph expansion, audio scene routing, or
business sound IDs.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `873`;
- `ctest --preset windows-fast-gate -N -L Audio`: `32`;
- `ctest --preset windows-fast-gate -N -L Fast`: `873`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `97`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `277`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Audio`: `1`.

## Approval Evidence

Approved after ENG-147A, ENG-147B, and ENG-147C proposal reviews PASS.

Review evidence:

- proposal commit `b9389eecef47ccb95ace032919c9ddc25e6fda07` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check b9389ee^..b9389ee` passed;
- review work was read-only and made no source, doc, commit, push, approval, or
  implementation changes;
- review confirmed no boundary, quality, implementability, test, or preset
  blocker;
- boundary review confirmed the proposal stays in Audio-owned PCM sample packet
  value contracts, bounded metadata, query/release snapshots, and deterministic
  counters;
- implementability review confirmed existing Audio fixed S16, stereo, 48000 Hz,
  slot/generation, snapshot/counter, and reject-before-mutation patterns are
  sufficient for the first slice;
- test-policy review confirmed deterministic default `windows-fast-gate`
  evidence, focused `Audio_PcmSamplePacket` or equivalent tests,
  CMakePresets no-drift, hardware-smoke isolation, and proof-shape scans are
  required;
- proposal discovery counts matched the reviewed baseline: default fast gate
  `873`, `Audio` `32`, `Fast` `873`, `PerformanceSmoke` `97`,
  `EvidenceOracle` `277`, default `HardwareSmoke` `0`, and
  `windows-hardware-smoke` `7` with `Audio` `1`.

## Owns

This gate owns the proposal for:

- Audio-owned PCM sample packet status, operation, request, record, and
  snapshot value contracts;
- fixed-capacity Audio-owned packet-slot metadata for caller-provided S16
  stereo PCM packets;
- validation for packet id, generation, sample format, sample rate, channel
  count, frame count, sample count, byte count, duplicate packet id, slot
  capacity, and release state;
- query and release behavior for packet metadata;
- deterministic snapshots for accepted, rejected, capacity rejected, duplicate,
  query, release, and stale-handle outcomes;
- proof that rejected packet requests do not mutate Audio voice, source,
  callback, Resource, File, Package, RHI, RenderCore, UI, World, Script, or
  Game Adapter state;
- deterministic tests under default `windows-fast-gate` with `Audio`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- audio file loading, package lookup, mounted file reads, archive parsing,
  original package readers, compression, codec parsing, real decode, resample,
  streaming, async decode, or Resource-backed asset lifetime;
- BGM/SE business IDs, original sound names, audio scene routing, UI sound
  behavior, Script audio services, gameplay audio, World behavior, reports,
  screenshots, logs, sleeps, manual listening proof, audible proof, or Game
  Adapter behavior;
- RHI upload, RenderCore scheduling, render graph, material graph, scene
  traversal, GPU resource lifetime, backend-native public headers, or hardware
  device proof;
- changing the existing real callback hardware-smoke policy or making default
  `windows-fast-gate` depend on an audio endpoint.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Audio owns sample format vocabulary, packet metadata, explicit packet
  lifecycle, source/voice mix contracts, callback submission values, and
  deterministic packet counters;
- Resource later owns decoded-result metadata and asset identity over File and
  Package boundaries;
- a later Audio-Resource bridge may connect Resource decode-result records to
  Audio packet requests after both sides are implemented and reviewed;
- UI, Script, World, and Game Adapter later own business sound routing and
  gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep Audio packet ownership, asset
decode, Resource lifetime, streaming, and gameplay audio separate.

## Dependencies

Allowed dependencies:

- existing public `YuAudio` backend, status, sample format, source, voice,
  callback descriptor, callback snapshot, and mixer value contracts;
- new Audio-owned PCM sample packet status, request, record, operation, and
  snapshot value contracts;
- private `YuAudio` implementation storage for bounded packet metadata;
- `Tests/Audio` deterministic tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuResource`, `YuStreaming`, `YuPackage`, `YuFile`, `YuRHI`, `YuRenderCore`,
  Material, World, Script, UI, Game Adapter, parser, real decoder, report,
  screenshot, visual proof, or original-game evidence dependencies from
  production Audio packet code;
- direct Windows SDK, XAudio2, WASAPI, COM, Platform, RHI, RenderCore,
  Resource, File, or Package types in public Audio PCM packet headers;
- generated reports, screenshots, logs, sleep timing, manual listening,
  audible output, hardware-only proof, or original-game output as evidence.

## Public Contract Boundary

Public Audio contracts may expose value-only descriptors such as:

- PCM sample packet status and operation values;
- packet id, generation, sample format, sample rate, channel count, frame
  count, interleaved sample count, byte count, and packet slot values;
- bounded snapshots for packet counters and last operation;
- query functions that return packet metadata without exposing file paths,
  package entries, Resource handles, parser state, decoded object pointers, or
  backend-private audio objects.

Public Audio contracts must not expose:

- Windows headers, XAudio2, WASAPI, COM, endpoint handles, callback objects,
  thread handles, native buffers, Resource handles, File paths, Package ids,
  RHI handles, RenderCore pass ids, material ids, scene ids, UI ids, report
  handles, screenshot artifacts, visual-proof types, or Game Adapter types;
- codec bitstream formats, package archive internals, parser objects, cache
  file paths, decoded asset object pointers, or original-game package layout
  details.

Public `YuAudio` headers must remain free of Resource, Streaming, Package,
File, RHI, RenderCore, material graph, scene, UI, World, Script, and Game
Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Audio-owned value contracts for PCM sample packet status,
   operation, request, record, and snapshot.
2. Add bounded Audio storage for packet-slot metadata and deterministic
   generation/release state.
3. Validate packet id, sample format, sample rate, channel count, frame count,
   sample count, byte count, duplicate packet id, capacity, stale handle, and
   release state before mutation.
4. Reject invalid format, unsupported sample rate, unsupported channel count,
   zero frame count, sample-count mismatch, byte-count mismatch, duplicate
   packet id, capacity overflow, stale handle, and released packet queries
   without mutating Audio source, voice, callback, Resource, File, Package, RHI,
   RenderCore, UI, World, Script, or Game Adapter state.
5. Add query and release behavior that returns or clears only Audio-owned
   packet metadata.
6. Keep existing `TestAudioDevice`, mixer, and callback behavior compatible and
   covered by regression tests.
7. Keep Audio core free of Resource, Streaming, Package, File, RHI, RenderCore,
   material graph, render graph, scene, UI, World, Script, and Game Adapter
   dependencies.
8. Add deterministic fast tests for packet create, query, release, duplicate
   id, invalid format, invalid sample rate, invalid channel count, zero frame
   count, sample-count mismatch, byte-count mismatch, capacity overflow, stale
   handle, no mutation on rejection, and snapshot counters.
9. Keep all required proof in default `windows-fast-gate`; no new
   hardware-smoke admission is required for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
audio hardware, audio files, original-game packages, screenshots, reports,
logs, sleeps, audible output, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `Audio_PcmSamplePacket` or equivalent tests proving the new boundary;
- discovery counts for `Audio`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`,
  and `HardwareSmoke`;
- proof that default `windows-fast-gate` still discovers `HardwareSmoke` as
  `0`;
- proof that existing `windows-hardware-smoke` remains isolated and that no new
  Audio packet proof depends on a real endpoint;
- public-header scan for native/backend/Resource/File/Package/RenderCore/World
  leakage;
- production dependency scan proving `YuAudio` packet code does not gain
  Resource, Streaming, Package, File, RHI, RenderCore, material graph, render
  graph, scene, UI, World, Script, Game Adapter, original-game parser, or real
  decode dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, audible proof, original-game outputs, hardware-only
  proof, and silent skip.

Accepted proof:

- deterministic assertions over public Audio PCM packet request, record, query,
  release, snapshot, and counter values;
- explicit capacity and validation counters proving failed paths do not mutate
  Audio packet state, source state, voice state, callback state, Resource state,
  File state, Package state, RHI state, RenderCore state, or upper-engine
  state;
- regression assertions that existing `Audio_` test backend, mixer, and
  callback fast tests remain deterministic.

Rejected proof:

- screenshots, reports, generated audio/image artifacts, manual listening,
  audible playback, logs, sleeps, visual demos, original-game packages, or
  original-game output;
- silent skip of Audio PCM packet proof;
- direct XAudio2/Win32/native proof as the only acceptance path;
- Resource load, File read, Package parse, codec decode, streaming, BGM/SE
  service, UI, World, Script, Game Adapter, RenderCore scheduling, RHI upload,
  or hardware-only behavior as evidence for this gate.

## Non Goals

- No audio file decoder, codec parser, or resampler.
- No File IO, mounted archive read, Package parsing, or original package
  reader.
- No Resource handle, Resource decode-result dependency, cache ownership, or
  asset lifetime.
- No streaming audio, async decode, ring buffer, mixer graph, panning,
  envelopes, effects, spatialization, or audio scene.
- No BGM/SE IDs, original sound names, UI sound behavior, Script audio service,
  World behavior, gameplay, report, screenshot, log, sleep, manual listening
  proof, audible proof, or Game Adapter behavior.
- No RHI upload, RenderCore scheduling, material graph, render graph, frame
  graph, or backend-native public exposure.
- No default fast-gate real-device requirement.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after a combined lower-engine review
confirms the proposal stays in Audio-owned PCM packet value contracts and
bounded packet metadata, remains implementable with the landed Audio test
backend and callback surfaces, preserves deterministic fast-gate proof, and
does not introduce Resource, File, Package, real codec, streaming, UI, World,
Script, Game Adapter, report, screenshot, log, sleep, manual, audible,
original-game, or hardware-only evidence.
