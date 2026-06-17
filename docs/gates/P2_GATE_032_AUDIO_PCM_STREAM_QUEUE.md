# P2-GATE-032: Audio PCM Stream Queue

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NOT_APPROVED`
Owner: 博丽灵梦
Reviewers: Combined lower-engine review
Depends on: P2-GATE-028, P2-GATE-002, P2-GATE-011, P2-GATE-004, ADR-0012
Related decisions: ADR-0012
Source baseline: `7204e9b`
Proposal commit: Pending review commit

## Layer

L3 lower-engine Audio stream-queue boundary over landed Audio PCM sample packet
value contracts.

This gate proposes the first bounded Audio-owned PCM stream queue. The first
slice may queue a validated frame range from an active `AudioPcmSamplePacket`,
drain deterministic chunk descriptors into caller-owned output storage, expose
query and release snapshots, and prove counter behavior without copying decoded
bytes, reading files, decoding codecs, creating Resource ownership, submitting
to the callback backend, or creating streaming audio, BGM/SE, scene, UI, World,
Script, or Game Adapter behavior.

```text
landed Audio PCM sample packet metadata
-> Audio-owned fixed-capacity PCM stream queue record
-> deterministic chunk descriptor drain into caller-owned storage
-> later bridge, byte-handoff, callback, or streaming gates
```

## Current Reality

P2-GATE-028 landed Audio-owned PCM sample packet value contracts at `534d3ef`.
Audio can validate caller-provided fixed S16 stereo packet descriptors, record
bounded packet-slot metadata, query and release packet records, and expose
deterministic counters without Resource, File, Package, codec, or streaming
ownership.

P2-GATE-029 landed a bridge-owned metadata mapping from Resource decode-result
records into Audio PCM packet request descriptors at `0a22dac`. That bridge
does not move Resource ownership into Audio core, does not store decoded bytes,
and does not own Audio packet lifecycle inside Resource core.

P2-GATE-031 is approved at `7204e9b` as a Resource decoded-payload ownership
gate. It may add Resource-owned decoded byte payload storage, but it explicitly
does not authorize Audio lifecycle, streaming audio, callback submission, or
Audio packet ownership changes.

The missing Audio boundary is a queue over already landed Audio PCM packet
metadata. Today Audio can create a packet record and query its frame/sample
shape, but it has no reviewed Audio-owned queue that records a packet frame
range, drains bounded chunk descriptors, invalidates queue handles on release,
and proves rejected queue operations do not mutate packet, source, voice, or
callback state.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `914`;
- `ctest --preset windows-fast-gate -N -L Audio`: `53`;
- `ctest --preset windows-fast-gate -N -L AudioResource`: `8`;
- `ctest --preset windows-fast-gate -N -L Fast`: `914`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `112`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `318`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Audio`: `1`.

## Owns

This gate owns the proposal for:

- Audio-owned PCM stream queue status, operation, request, handle, record,
  chunk descriptor, and snapshot value contracts;
- fixed-capacity Audio-owned queue records over active `AudioPcmSamplePacket`
  metadata;
- validation for queue id, packet handle, packet generation, expected packet id,
  sample format, sample rate, channel count, first frame, frame count,
  interleaved sample count, byte count, chunk frame count, duplicate queue id,
  queue capacity, output descriptor capacity, and release state;
- deterministic drain behavior that writes only chunk descriptors into
  caller-owned output storage;
- query and release behavior for queue metadata only;
- deterministic snapshots for accepted, rejected, duplicate, stale, packet,
  range, sample-count, byte-count, chunk, capacity, output-capacity, drain,
  query, and release outcomes;
- proof that rejected queue requests do not mutate Audio packet state, source
  state, voice state, callback state, Resource, File, Package, Streaming, RHI,
  RenderCore, UI, World, Script, or Game Adapter state;
- deterministic tests under default `windows-fast-gate` with `Audio`, `Fast`,
  `PerformanceSmoke`, and `EvidenceOracle` labels.

## Does Not Own

This gate does not own:

- decoded audio byte storage, decoded byte copying, byte handoff from Resource,
  Resource handles, AudioResource bridge behavior, real codec output, resample,
  streaming audio, async decode, ring buffers, or asset lifetime policy;
- callback device submission, XAudio2 buffer submission, endpoint scheduling,
  hardware proof, audible playback, manual listening, or real-device proof;
- audio file loading, package lookup, mounted file reads, archive parsing,
  original package readers, compression, parser state, or original-game output;
- BGM/SE business ids, original sound names, audio scene routing, UI sound
  behavior, Script audio services, gameplay audio, World behavior, reports,
  screenshots, logs, sleeps, manual proof, or Game Adapter behavior;
- RHI upload, RenderCore scheduling, render graph, material graph, scene
  traversal, GPU resource lifetime, backend-native public headers, or hardware
  device admission.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Audio PCM sample packets own fixed-format packet metadata and packet
  lifecycle;
- this proposed queue owns only ordered frame-range queue metadata and chunk
  descriptor drain over those packet records;
- AudioResource bridges own cross-module metadata mapping from Resource
  decode-result records into Audio packet request descriptors;
- Resource decoded payloads own Resource-controlled decoded bytes after a
  separate Resource gate;
- later callback, streaming, byte-handoff, mixer, scene, and gameplay gates may
  consume these records after separate review.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep packet metadata, stream queue
metadata, decoded byte ownership, callback submission, streaming, and gameplay
audio separate.

## Dependencies

Allowed dependencies:

- existing public `YuAudio` status, sample format, PCM sample packet request,
  record, handle, operation, snapshot, source, voice, callback descriptor, and
  mixer value contracts;
- new Audio-owned PCM stream queue status, operation, request, handle, record,
  chunk descriptor, and snapshot value contracts;
- private `YuAudio` implementation storage for bounded queue metadata;
- `Tests/Audio` deterministic tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- `YuResource`, `YuAudioResource`, `YuStreaming`, `YuPackage`, `YuFile`,
  `YuRHI`, `YuRenderCore`, Material, World, Script, UI, Game Adapter, parser,
  real decoder, report, screenshot, visual proof, or original-game evidence
  dependencies from production Audio stream queue code;
- direct Windows SDK, XAudio2, WASAPI, COM, Platform, RHI, RenderCore,
  Resource, File, Package, or backend-native types in public Audio stream queue
  headers;
- generated reports, screenshots, logs, sleep timing, manual listening,
  audible output, hardware-only proof, or original-game output as evidence.

## Public Contract Boundary

Public Audio stream queue contracts may expose value-only descriptors such as:

- PCM stream queue status and operation values;
- queue id, queue slot, queue generation, packet handle, packet id, packet
  generation, sample format, sample rate, channel count, first frame, frame
  count, drained frame count, remaining frame count, interleaved sample count,
  byte count, chunk frame count, and final-chunk values;
- caller-owned chunk descriptor output storage and descriptor counts;
- bounded snapshots for queue counters and last operation;
- query functions that return queue metadata without exposing file paths,
  package entries, Resource handles, decoded byte storage, parser state,
  callback backend objects, endpoint handles, or backend-private audio objects.

Public Audio stream queue contracts must not expose:

- Windows headers, XAudio2, WASAPI, COM, endpoint handles, callback objects,
  thread handles, native buffers, Resource handles, File paths, Package ids,
  RHI handles, RenderCore pass ids, material ids, scene ids, UI ids, report
  handles, screenshot artifacts, visual-proof types, or Game Adapter types;
- codec bitstream formats, package archive internals, parser objects, cache
  file paths, decoded asset object pointers, ring buffer internals, or
  original-game package layout details.

Public `YuAudio` stream queue headers must remain free of Resource,
AudioResource, Streaming, Package, File, RHI, RenderCore, material graph,
scene, UI, World, Script, and Game Adapter dependencies.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only Audio-owned value contracts for PCM stream queue status, operation,
   request, handle, record, chunk descriptor, and snapshot.
2. Add bounded Audio storage for stream queue records and deterministic
   generation/release state.
3. Validate queue id, packet handle, packet generation, expected packet id,
   packet active state, sample format, sample rate, channel count, first frame,
   frame count, frame-range overflow, interleaved sample count, byte count,
   chunk frame count, duplicate queue id, queue capacity, descriptor output
   capacity, stale handle, and release state before mutation.
4. Reject invalid packet handles, released packet handles, packet id mismatch,
   unsupported format, unsupported sample rate, unsupported channel count,
   empty frame ranges, frame-range overflow, sample-count mismatch,
   byte-count mismatch, invalid chunk frame count, duplicate queue id, queue
   capacity overflow, output descriptor capacity overflow, stale handles, and
   released queue queries without mutating Audio packet, source, voice,
   callback, Resource, File, Package, Streaming, RHI, RenderCore, UI, World,
   Script, or Game Adapter state.
5. Add drain behavior that writes deterministic chunk descriptors only into
   caller-owned output storage and never copies or owns decoded bytes.
6. Add query and release behavior that returns or clears only Audio-owned queue
   metadata.
7. Keep existing `TestAudioDevice`, packet, mixer, and callback behavior
   compatible and covered by regression tests.
8. Keep Audio core free of Resource, AudioResource, Streaming, Package, File,
   RHI, RenderCore, material graph, render graph, scene, UI, World, Script,
   and Game Adapter dependencies.
9. Add deterministic fast tests for queue create, query, drain, release,
   invalid packet handle, released packet handle, packet id mismatch, duplicate
   queue id, frame-range overflow, sample-count mismatch, byte-count mismatch,
   invalid chunk frame count, capacity overflow, small output capacity,
   stale handle, no mutation on rejection, and snapshot counters.
10. Keep all required proof in default `windows-fast-gate`; no new
    hardware-smoke admission is required for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
audio hardware, audio files, original-game packages, screenshots, reports,
logs, sleeps, audible output, decoded asset bytes, or manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- focused `Audio_PcmStreamQueue` or equivalent tests proving the new boundary;
- discovery counts for `Audio`, `AudioResource`, `Fast`, `PerformanceSmoke`,
  `EvidenceOracle`, and `HardwareSmoke`;
- proof that default `windows-fast-gate` still discovers `HardwareSmoke` as
  `0`;
- proof that existing `windows-hardware-smoke` remains isolated and that no new
  stream queue proof depends on a real endpoint, decoded asset, or audible
  output;
- public-header scan for native/backend/Resource/AudioResource/File/Package/
  RenderCore/World leakage;
- production dependency scan proving `YuAudio` stream queue code does not gain
  Resource, AudioResource, Streaming, Package, File, RHI, RenderCore, material
  graph, render graph, scene, UI, World, Script, Game Adapter, original-game
  parser, real decode, or real streaming dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, generated artifacts, logs,
  sleeps, manual proof, audible proof, original-game outputs, hardware-only
  proof, and silent skip.

Accepted proof:

- deterministic assertions over public Audio PCM stream queue request, record,
  chunk descriptor, query, drain, release, snapshot, and counter values;
- explicit capacity and validation counters proving failed paths do not mutate
  Audio queue state, packet state, source state, voice state, callback state,
  Resource state, File state, Package state, RHI state, RenderCore state, or
  upper-engine state;
- regression assertions that existing `Audio_PcmSamplePacket`, `Audio_` test
  backend, mixer, and callback fast tests remain deterministic.

Rejected proof:

- screenshots, reports, generated audio/image artifacts, manual listening,
  audible playback, logs, sleeps, visual demos, original-game packages, or
  original-game output;
- silent skip of Audio PCM stream queue proof;
- direct XAudio2/Win32/native proof as the only acceptance path;
- Resource load, AudioResource bridge execution, File read, Package parse,
  codec decode, decoded byte storage, streaming, BGM/SE service, UI, World,
  Script, Game Adapter, RenderCore scheduling, RHI upload, or hardware-only
  behavior as evidence for this gate.

## Non Goals

- No audio file decoder, codec parser, decoded byte storage, byte copying,
  resampler, async decode, or streaming buffer.
- No File IO, mounted archive read, Package parsing, or original package
  reader.
- No Resource handle, Resource decoded-payload dependency, AudioResource bridge
  execution, cache ownership, or asset lifetime.
- No automatic callback submission, XAudio2 buffer submission, hardware device
  proof, or audible playback.
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

Request `APPROVED_FOR_FIRST_SLICE` only after combined lower-engine review
confirms the proposal stays in Audio-owned PCM stream queue metadata over
landed Audio PCM sample packet contracts, remains implementable with current
public value contracts, preserves deterministic fast-gate proof, and does not
introduce Resource, AudioResource, decoded bytes, real codec, streaming,
callback submission, BGM/SE, UI, World, Script, Game Adapter, report,
screenshot, log, sleep, manual, audible, hardware-only, or original-game
evidence.
