# P2-GATE-002: Audio Test Backend And Mixer Sink

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Depends on: ADR-0012
Related decisions: ADR-0002, ADR-0005, ADR-0006
Source baseline: Phase 2 through `2bf993f`

## Layer

L3 Audio backend/mixer boundary.

This gate proves a deterministic test backend and caller-driven mixer sink
without real audio devices, callback threads, codecs, streaming, resources,
BGM/SE business IDs, script/UI/gameplay audio, or Game Adapter behavior.

## Owns

This gate owns the first `YuAudio` implementation slice for:

- audio backend kind vocabulary;
- test backend device;
- device capabilities;
- fixed signed PCM format;
- bounded synthetic sample source registration;
- generation-checked voice handles;
- bounded voice table;
- start/stop voice behavior;
- caller-driven mix into caller-owned buffers;
- explicit Audio result/status values;
- deterministic counters for tests.

## Does Not Own

This gate does not own:

- WASAPI, XAudio2, DirectSound, OpenAL, SDL, FMOD, Wwise, or real device backend;
- audio callback thread, mixer thread, streaming thread, or async decode;
- codec parser/decoder or sample import;
- `YuFile` reads or `YuResource` handles;
- BGM/SE business IDs or TouhouNewWorld audio service names;
- audio scene, UI sound routing, script audio services, or gameplay audio;
- report/profiler/capture/oracle/dashboard/editor output.

## UE/Unity Analogue

UE5 references:

- `Runtime\AudioMixer`, `Runtime\AudioMixerCore`, and audio codec/platform
  modules as responsibility references.

Unity references:

- Audio system, AudioSource, AudioMixer, and audio asset docs as workflow
  references.

YuEngine decision:

- Start with a deterministic test backend and mixer sink.
- Keep real device callbacks, resource decode/streaming, audio scene, and
  game-specific sound IDs outside the first slice.
- Count audio progress only when voice lifecycle and mixed sample output are
  testable without OS audio or original assets.

## Lifecycle

First-slice lifecycle:

1. Setup creates a test audio device with fixed capacities.
2. Setup registers bounded synthetic sample sources.
3. Setup starts one or more voices from registered sources.
4. Mix request writes a bounded number of stereo `S16` frames into caller-owned
   output storage.
5. Mixing advances voice cursors deterministically and stops voices at
   end-of-source.
6. Stop destroys voice activity and increments generation so stale handles fail.
7. Shutdown stops all voices and releases test backend state.

Failure behavior:

- unsupported backend kind returns explicit unsupported status;
- unsupported format, sample rate, or channel count returns explicit format
  status and does not mutate device state;
- source capacity overflow returns explicit capacity status and does not mutate
  source state;
- voice capacity overflow returns explicit capacity status and does not mutate
  voice state;
- invalid or stale voice handle returns explicit handle status;
- start with missing source returns explicit not-found status;
- invalid gain outside `[0, 32767]` returns explicit invalid-value status and
  does not mutate voice state;
- accepted mix requests overwrite every requested output frame/channel, and
  frames/channels with no contributing active voice are written as silence
  (`0`);
- mix with an undersized output buffer returns explicit capacity status, leaves
  destination samples unchanged, and reports `frames_written == 0`;
- disabled diagnostics/logging does not change any Audio result.

## Inputs

- test backend device descriptor;
- fixed `S16`, `48000 Hz`, stereo format;
- synthetic interleaved source samples;
- voice start requests with Q15 gain;
- caller-owned output buffers;
- optional memory tracker if P1-GATE-002 implementation is accepted.

## Outputs

- audio device capabilities;
- source handles or IDs;
- voice handles;
- Audio result/status values;
- mixed interleaved `S16` samples in caller-owned storage;
- deterministic counters for registered sources, active voices, mixed frames,
  stopped voices, failed operations, and allocation/accounting status.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` for accounting vocabulary/signal tests when available and accepted;
- `YuDiagnostics` only for disabled-behavior observation when available.

Target dependency expectation:

```text
YuAudio
  -> optional YuMemory for accounting vocabulary/signal tests
  -> optional YuDiagnostics for disabled-behavior observation
```

`YuAudio` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`,
`YuResource`, RHI, input, script, scene/world, UI, tools, reports, or
TouhouNewWorld evidence in this first slice.

## Performance Constraints

Required deterministic signals:

- source capacity;
- voice capacity;
- output frame capacity;
- active voice count;
- mixed frame count;
- failed operation count;
- output buffer write count;
- voice generation/stale handle count;
- voice storage capacity before/after mix fixture;
- allocation/accounting status using `YuMemory` vocabulary or explicit
  deferral;
- disabled diagnostics/logging behavior equivalence.

First-slice bounds:

- backend kinds: `Test` only;
- sample format: signed 16-bit PCM only;
- sample rate: `48000` only;
- channels: stereo only;
- synthetic source capacity: 8 sources maximum;
- voice capacity: 16 voices maximum;
- source frame capacity: 256 stereo frames per source maximum;
- output mix request: 64 stereo frames maximum;
- voice gain: unsigned Q15 `[0, 32767]`;
- Q15 sample scaling formula:
  `(source_sample * gain) / 32767`, where `source_sample` is converted to signed
  32-bit first, multiplication uses signed 32-bit intermediate arithmetic, and
  integer division rounds toward zero;
- unity gain `32767` must preserve every signed 16-bit source sample exactly,
  including `-32768`;
- accepted mix requests overwrite every requested output frame/channel; existing
  destination contents are not accumulated into the mix result;
- requested frames/channels with no contributing active voice, including tail
  after end-of-source, are written as silence (`0`);
- mix execution must not allocate or grow storage.

Pass/fail rule:

- exceeding source, voice, source-frame, or output-frame bounds is an explicit
  failure;
- changing voice storage capacity, allocating, reading files, or depending on
  diagnostics/report/profiler/oracle output during the mix fixture is a gate
  failure unless this gate is amended.

Blocking conditions:

- real audio device backend or callback thread;
- file/resource read, decoder, streaming, or sample cache ownership;
- BGM/SE ID, script service, scene, UI, gameplay, or Game Adapter semantics;
- unbounded source table, voice table, or output buffer;
- hidden allocation in measured mix path;
- diagnostics/log/report output required for behavior;
- tests that validate behavior by parsing logs, reports, profiler output, audio
  files, or original resources.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Audio_CreateTestDevice_ReturnsCapabilities`
- `Audio_CreateDevice_RejectsUnsupportedBackend`
- `Audio_CreateDevice_RejectsUnsupportedFormat`
- `Audio_RegisterSyntheticSource_ReturnsStableId`
- `Audio_SourceCapacityOverflow_DoesNotMutate`
- `Audio_StartVoice_ReturnsGenerationHandle`
- `Audio_StartVoice_RejectsMissingSource`
- `Audio_StartVoice_RejectsInvalidGainWithoutMutation`
- `Audio_VoiceCapacityOverflow_DoesNotMutate`
- `Audio_StopVoice_InvalidatesStaleHandle`
- `Audio_MixSingleVoice_WritesDeterministicS16StereoSamples`
- `Audio_MixUnityGain_PreservesS16EdgeSamples`
- `Audio_MixFractionalGain_RoundsTowardZeroDeterministically`
- `Audio_MixMultipleVoices_UsesStableOrderAndSaturates`
- `Audio_MixStopsVoiceAtEndOfSource`
- `Audio_MixEndedVoice_WritesSilentTail`
- `Audio_MixOverwritesPrefilledDestination`
- `Audio_MixRejectsUndersizedBufferWithoutWritingSamples`
- `Audio_Mix_DoesNotGrowVoiceStorage`
- `Audio_DisabledDiagnosticsDoesNotChangeResults`
- `Audio_NoDeviceCodecResourceScriptUiGameAdapterDependency`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

The implementation handoff must record the exact commands used.

## Allowed First Slice

If approved, the first implementation slice may create:

```text
src/yuengine/audio/include/yuengine/audio/
src/yuengine/audio/src/
tests/audio/
```

It may update root `CMakeLists.txt` only to add `YuAudio` and `YuAudioTests`.

It may not create placeholder directories or targets for real audio backends,
codecs, streaming, resource upload, audio scene, UI, script, gameplay, tools,
report, profiler, capture, oracle, or Game Adapter work.

## Non-Goals

- No real audio backend or callback thread.
- No audio file decoder.
- No streaming.
- No sample-rate conversion.
- No panning, envelopes, effects, or mixer graph.
- No Resource/File integration.
- No BGM/SE IDs or original audio service names.
- No UI/script/gameplay audio behavior.
- No report/profiler/oracle output.

## Evidence Inputs

No original-game evidence is required for the first slice.

TouhouNewWorld audio service calls, BGM/SE IDs, audio files, codec facts, and old
backup audio reports remain future validation evidence only. They must not be
read by P2-GATE-002 fast tests.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0012 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements and
  sequencing against active Phase 1/2 review work;
- 八云蓝 confirms the UE5/Unity boundary comparison is sound;
- 博丽灵梦 confirms the caller-driven mix cost model and no-allocation mix path;
- 大妖精 confirms the public surface and tests are locally implementable;
- 射命丸文 confirms original audio evidence is not being used as API shape if
  evidence boundary is questioned.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_PERFORMANCE`, or `NEEDS_EVIDENCE` with exact missing fields.
