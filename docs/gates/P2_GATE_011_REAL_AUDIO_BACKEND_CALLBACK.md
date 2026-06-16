# P2-GATE-011: Real Audio Backend Callback

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NOT_APPROVED`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-002, P2-GATE-004, P2-GATE-010, ADR-0012
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0012
Source baseline: `5a26a53`

## Layer

L1-L3 lower-engine audio backend proof.

This gate proposes the first real audio backend callback slice after the landed
test backend/mixer sink and worker/async substrate. It is not an audio scene,
codec, streaming, Resource, UI, script, gameplay, or Game Adapter gate. The
goal is to prove that YuAudio can own a real platform callback backend without
letting OS device details, callback threads, or endpoint policy leak into the
public mixer boundary.

```text
existing deterministic TestAudioDevice and mixer sink
-> explicit real backend descriptor and status vocabulary
-> private Windows callback backend implementation
-> fixed silent or synthetic S16 buffer submission
-> callback completion counters and shutdown proof
-> optional hardware-smoke execution isolated from default fast gate
-> later decode, streaming, audio scene, and Resource-backed audio gates
```

## Current Reality

P2-GATE-002 landed a deterministic `YuAudio` test backend:

- `TestAudioDevice`;
- audio backend, format, status, descriptor, capability, source, voice, mix, and
  snapshot value types;
- synthetic S16 stereo sources;
- generation-checked voice handles;
- caller-driven mix into caller-owned buffers;
- deterministic `Audio_` fast tests;
- no real audio device, callback thread, codec, file read, streaming,
  Resource, UI, script, gameplay, or Game Adapter behavior.

P2-GATE-010 landed worker lifecycle and async file-completion proof at
`5a26a53`, but it explicitly excluded real audio callback behavior. That means
the lower-engine substrate can now prove bounded background work, while Audio
still has only a test sink.

## Owns

This gate owns the proposal for:

- first real audio backend kind and explicit unavailable/unsupported statuses;
- a private Windows callback backend implementation using WASAPI, XAudio2, or a
  narrower Windows audio API chosen during implementation review;
- callback lifecycle values for initialize, start, submit, callback completion,
  stop, drain, and shutdown;
- fixed-format S16 stereo, `48000 Hz`, bounded callback buffers;
- caller-owned or backend-owned fixed callback buffer slots with declared
  capacity;
- deterministic counters for submitted buffers, completed callbacks, failed
  submissions, shutdown callbacks, underruns, and max queued buffers;
- optional hardware-smoke test that proves callback completion through counters
  and statuses, not audible playback;
- fast deterministic tests for public value contracts and dependency boundaries;
- CTest labels that keep real-device proof out of the default
  `windows-fast-gate`.

## Does Not Own

This gate does not own:

- audio codecs, file parsing, decode, decompression, sample import, resampling,
  spatialization, effects, mixer graph, or audio scene routing;
- `YuFile`, `YuResource`, or `YuPackage` integration for audio assets;
- BGM/SE business IDs, original-game sound names, TouhouNewWorld audio services,
  UI/menu audio behavior, script audio services, gameplay rules, World, or Game
  Adapter behavior;
- streaming audio, ring-buffer streaming from disk, async decode, or cache
  ownership;
- general thread pool behavior, Resource upload, RHI, RenderCore, reports,
  profiler UI, screenshots, manual listening, or visual proof;
- making the default `windows-fast-gate` require a real audio endpoint.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- `YuAudio` owns backend vocabulary, mixer contracts, voice handles, callback
  lifecycle, buffer submission, and callback counters;
- the private Windows audio backend owns platform API calls and platform
  callback objects;
- `YuThread` owns general worker lifecycle, but this first audio callback slice
  must not reuse worker jobs as the audio callback proof;
- `YuFile`, `YuResource`, and `YuPackage` own later audio asset read, decode,
  load, and lifetime behavior;
- UI, Script, World, and Game Adapter own later business routing and gameplay
  semantics.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep backend callback ownership,
decode, streaming, resource lifetime, and gameplay audio separate.

## Dependencies

Allowed dependencies:

- existing `YuAudio` public and private files;
- existing test backend and mixer sink contracts;
- Windows audio SDK libraries in private Windows implementation files only;
- CMake/CTest labels and optional `windows-hardware-smoke` admission;
- test-only fixture files under `Tests/Audio`.

Forbidden dependencies:

- `YuFile`, `YuResource`, `YuPackage`, `YuRHI`, RenderCore, UI, Script, World,
  tools, reports, profiler UI, screenshots, visual proof, or Game Adapter from
  production `YuAudio`;
- Windows SDK, COM interfaces, platform endpoint handles, WASAPI interfaces,
  XAudio2 interfaces, or callback objects in public `YuAudio` headers;
- Thread wait primitives in public `YuAudio` headers;
- file reads, package lookup, resource handles, codec parsing, streaming, async
  decode, or sample cache behavior in tests or runtime code;
- proof that depends on logs, reports, screenshots, manual listening, or audible
  output.

## Public Surface Shape

The first slice should keep public API expansion small and value-based.
Suggested additions:

- extend `AudioBackendKind` with one explicit real backend value, such as
  `XAudio2` or `WindowsDefault`, if the implementation review accepts the name;
- extend `AudioStatus` with unavailable, device-start, buffer-submit, callback,
  and shutdown statuses as needed;
- add an `AudioCallbackDeviceDesc` or equivalent value type with fixed buffer
  count, frames per buffer, sample rate, channel count, format, and backend
  kind;
- add an `AudioCallbackSnapshot` or equivalent value type with submitted,
  completed, failed, underrun, queued, max queued, and shutdown counters;
- add an explicit owner object, such as `AudioCallbackDevice`, whose public
  methods are initialize, start, submit fixed S16 buffer, stop, drain, shutdown,
  and snapshot.

Public headers must not expose Windows headers, COM, XAudio2, WASAPI, endpoint
handles, callback class definitions, worker thread handles, Resource, Package,
File, RHI, RenderCore, UI, World, report, screenshot, visual proof, or Game
Adapter types.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller creates a callback descriptor with fixed backend, format, buffer count,
   and frames per buffer.
2. Caller initializes an explicit audio callback device owner.
3. Backend private code creates the platform device and callback object.
4. Caller starts the backend.
5. Caller submits a fixed S16 stereo silent or synthetic buffer.
6. Platform callback completes the submitted buffer and updates bounded counters.
7. Caller drains or snapshots callback completion state.
8. Caller stops the backend.
9. Caller shuts down the backend and releases platform objects.
10. Snapshot proves submitted/completed/failure/shutdown counts and stable
    capacities.

Failure behavior:

- unsupported backend returns explicit status without mutating initialized state;
- missing audio endpoint returns explicit unavailable status;
- invalid format, channel count, sample rate, buffer count, or frame count
  returns explicit descriptor status;
- submit before start returns explicit lifecycle status;
- submit after stop or shutdown returns explicit shutdown status;
- callback completion capacity overflow returns explicit status or bounded
  counter without overrun;
- stop and shutdown are idempotent;
- no-audio-device machines must fail the hardware-smoke proof explicitly or be
  reported as hardware-unavailable evidence, not silently counted as callback
  proof.

## Inputs

- fixed callback backend descriptor;
- fixed S16 stereo sample buffers;
- caller-owned or backend-owned fixed buffer slots;
- callback completion storage;
- optional real Windows audio endpoint for hardware-smoke proof.

## Outputs

- explicit Audio statuses;
- callback device snapshot counters;
- callback completion count;
- no audio files, resource handles, package entries, UI events, game sound IDs,
  reports, screenshots, or manual listening outputs.

## Test And Preset Strategy

Default `windows-fast-gate` remains deterministic and no-real-device:

- public value contract tests are labeled `Fast`, `ModuleFixture`, and `Audio`;
- dependency and proof-shape tests are labeled `EvidenceOracle`;
- capacity and no-growth tests are labeled `PerformanceSmoke`;
- no `HardwareSmoke`, `XAudio2`, `WASAPI`, or `Win32` labels are added to the
  default deterministic tests.

Optional real-device callback proof is isolated:

- add a separate `YuAudioHardwareSmokeTests` target only if implementation review
  confirms the local toolchain and runtime can support it;
- hardware tests are labeled `HardwareSmoke`, `Audio`, `Win32`, and the accepted
  backend label such as `XAudio2` or `WASAPI`;
- `windows-hardware-smoke` may discover and run the real callback test;
- default `windows-fast-gate -N -L HardwareSmoke` must remain `0`;
- the proof must use callback counters/statuses and bounded waits/signals, not
  sleeps, logs, reports, screenshots, manual listening, or audible output.

Expected deterministic first-slice growth is about 6 to 12 fast tests plus 0 to
2 optional hardware-smoke tests. The gate must not remove or relabel existing
deterministic `Audio_` tests to make the count look smaller.

## Performance Constraints

Required signals:

- declared callback buffer count;
- declared frames per callback buffer;
- submitted buffer count;
- completed callback count;
- failed submission count;
- underrun or starvation count, if observable;
- queued/max queued buffer counts;
- shutdown count and final lifecycle status;
- allocation count or explicit setup-only allocation proof;
- no dynamic growth during callback submission/completion fixture;
- no blocking file IO, package lookup, resource load, codec decode, report
  output, or lock-unbounded behavior in callback path.

Blocking conditions:

- public OS handle, COM, XAudio2, WASAPI, or callback object exposure;
- real callback proof requires manual listening or log parsing;
- default fast gate depends on an audio endpoint;
- callback thread performs file IO, Resource lookup, Package lookup, codec
  decode, report output, UI/script/gameplay dispatch, or Game Adapter behavior;
- callback path allocates or grows storage during the measured fixture;
- unbounded buffer queue or hidden global audio device owner;
- implementation makes test backend/mixer deterministic behavior depend on a
  real device.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate --output-on-failure`
- `ctest --preset windows-fast-gate -N -L Audio`
- `ctest --preset windows-fast-gate -N -L Fast`
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`
- `ctest --preset windows-hardware-smoke -N`
- `git diff --check`

If a hardware-smoke callback test is admitted, the implementation handoff must
also record:

- `ctest --preset windows-hardware-smoke --output-on-failure`;
- real backend test discovery count;
- explicit callback completion proof shape;
- unavailable-device behavior if the machine cannot open an endpoint.

The implementation handoff must record:

- deterministic discovery before and after;
- `Audio`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and `HardwareSmoke`
  discovery counts;
- proof that default `windows-fast-gate` stays deterministic;
- public-header dependency scan;
- production dependency scan for forbidden modules and platform types;
- callback lifecycle proof shape;
- regression evidence for existing test backend/mixer `Audio_` tests.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioBackendKind.h
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioStatus.h
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioDeviceDesc.h
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioDeviceSnapshot.h
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackDevice.h
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackDeviceDesc.h
Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackSnapshot.h
Src/YuEngine/Audio/Src/AudioCallbackDevice.cpp
Src/YuEngine/Audio/Src/AudioCallbackDeviceWindows.cpp
Tests/Audio/AudioTests.cpp
Tests/Audio/AudioHardwareSmokeTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_011_REAL_AUDIO_BACKEND_CALLBACK.md
```

`CMakePresets.json` is not expected to change. Any CMake change must preserve
the default `windows-fast-gate` behavior.

Implementation review may narrow file names, but it may not expand beyond
`YuAudio`, `Tests/Audio`, root CMake labels, and this gate/queue documentation
without returning `NEEDS_ARCHITECTURE`.

## Non-Goals

- No codec parser or decoder.
- No audio file loading.
- No Resource or Package integration.
- No streaming audio.
- No sample-rate conversion.
- No panning, envelopes, effects, spatialization, or mixer graph.
- No audio scene.
- No BGM/SE IDs or original audio service names.
- No UI, Script, World, gameplay, report, screenshot, manual listening proof, or
  Game Adapter.
- No default fast-gate real-device requirement.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the boundary keeps real audio backend callback ownership
  separate from codec, Resource, Package, UI, Script, World, Game Adapter, and
  report ownership;
- 博丽灵梦 confirms the proposed file layout and callback lifecycle are locally
  implementable without public OS handle exposure, hidden global device
  lifetime, unbounded buffers, callback-path allocation, or callback-path IO;
- 雾雨魔理沙 confirms the test and preset policy is enforceable, keeps default
  `windows-fast-gate` deterministic, isolates any real endpoint proof in
  `windows-hardware-smoke`, and does not rely on logs, sleeps, reports,
  screenshots, manual listening, or audible output;
- 八云紫 confirms this gate is the next lower-engine proposal after P2-GATE-010
  and before OS input bridge, Resource/Package streaming, audio codec/streaming,
  UI, World, or Game Adapter tasks.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.
