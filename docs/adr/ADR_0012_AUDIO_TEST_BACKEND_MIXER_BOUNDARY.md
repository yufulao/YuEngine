# ADR-0012: Audio Test Backend And Mixer Sink Boundary

Status: Proposed
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, ADR-0006, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md, docs/YUENGINE_PHASE0_SUBSYSTEM_REFERENCE_MAP.md

## Context

YuEngine needs an Audio boundary before audio scenes, UI sounds, BGM/SE business
IDs, script audio services, or TouhouNewWorld adapter behavior can be
implemented. The restart plan states that audio business state waits for audio
backend and mixer boundaries.

The first Audio decision therefore defines a deterministic test backend and
mixer sink. It proves voice handles, fixed-format sample sources, caller-driven
mixing, explicit statuses, and bounded output without opening a real audio
device, callback thread, codec, streaming, resource loading, or game-specific
sound semantics.

## Decision

YuEngine introduces `YuAudio` as the owner of:

- audio backend vocabulary;
- test backend device identity and capabilities;
- fixed audio format descriptors;
- generation-checked voice handles;
- bounded synthetic sample sources;
- caller-driven mixer state;
- deterministic mix output into caller-owned buffers;
- explicit status values for invalid descriptors, invalid handles, capacity
  failures, and output-buffer failures.

The first Audio slice is a test backend and mixer sink only. It must not open a
real audio device, spawn an audio callback thread, decode files, stream data,
read resources, define BGM/SE IDs, or route script/UI/gameplay audio behavior.

## Audio Model

Initial concepts:

```text
AudioBackendKind
AudioDeviceDesc
AudioCapabilities
AudioFormat
AudioSampleSource
AudioVoiceHandle
AudioVoiceState
AudioMixRequest
AudioMixResult
AudioStatus
```

Names may change during implementation review, but the responsibilities must
stay equivalent.

Rules:

- `Test` is the only approved backend in P2-GATE-002;
- sample format is signed 16-bit PCM (`S16`);
- sample rate is fixed at `48000`;
- channel count is fixed at stereo (`2`);
- samples are interleaved left/right frames;
- synthetic sample sources are setup-time arrays, not streamed data;
- mixer output writes into caller-owned interleaved `S16` buffers;
- voice handles use slot/index plus generation, not raw pointers;
- stale-generation handles fail explicitly;
- disabled diagnostics/logging does not change any Audio result.

## Mixing Semantics

P2-GATE-002 mixing is deterministic integer mixing.

Rules:

- voice gain is unsigned Q15 fixed-point in `[0, 32767]`;
- `32767` means unity gain and `0` means silence;
- source sample multiplied by gain uses integer arithmetic with deterministic
  rounding toward zero;
- active voice samples are accumulated in signed 32-bit integers per output
  channel;
- final output is saturated to signed 16-bit range `[-32768, 32767]`;
- voices are mixed in stable voice-slot order;
- if a voice reaches end-of-source during a mix, it becomes stopped and stops
  contributing further samples;
- no resampling, panning, channel conversion, looping, envelopes, effects, or
  graph processing is introduced.

Future gates may add float formats, resampling, spatialization, effects, mixer
graphs, streaming, decode, and real callback/device behavior only after this
deterministic sink is accepted.

## Backend Boundary

The test backend is a deterministic sink, not a real device abstraction.

Blocked:

- WASAPI, XAudio2, DirectSound, OpenAL, SDL, FMOD, Wwise, or other real audio
  device backends;
- OS callback threads;
- device hotplug;
- clock drift, latency negotiation, or exclusive/shared mode policy;
- blocking IO in mixer or callback paths.

Future real backend gates must define callback lifetime, synchronization, buffer
queue ownership, underrun policy, and deterministic shutdown separately.

## Resource And Codec Boundary

P2-GATE-002 must not load audio files.

Blocked:

- `YuFile` reads;
- `YuResource` handles;
- WAV/OGG/MP3/ADX or original format parsing;
- streaming, decode, decompression, or sample cache ownership;
- TouhouNewWorld sound names, BGM/SE IDs, script audio service IDs, or original
  resource lookup.

Synthetic sample arrays in tests are generic fixtures only. Future Resource and
Audio gates will define asset lifetime, decode, and streaming separately.

## Diagnostics Boundary

Diagnostics may observe Audio counters later. Diagnostics must not own Audio
behavior.

Rules:

- Audio results are explicit status values.
- Tests observe statuses, counters, and mixed sample bytes, not logs.
- Disabled diagnostics/logging does not change device, voice, or mix results.
- Reports, profiler captures, oracle records, or dashboards are not Audio
  runtime APIs in this slice.

## Memory And Thread Boundary

Memory:

- device, source, voice, and mix buffer capacities are explicit;
- mix execution must not allocate or grow storage;
- if `YuMemory` is available and accepted, Audio allocation/accounting signals
  use its vocabulary;
- if `YuMemory` is unavailable or under blocking rewrite, accounting is
  explicitly deferred and cannot be counted as zero CRT/STL/general heap cost.

Thread:

- P2-GATE-002 is single-threaded and caller-driven;
- no audio callback thread, mixer thread, worker queue, streaming job, or async
  decode task is introduced;
- future real-device work must define synchronization and deterministic shutdown
  separately.

## Evidence Boundary

TouhouNewWorld audio service calls, BGM/SE IDs, audio resources, codec
observations, and old backup audio reports are future validation inputs. They
are not inputs to P2-GATE-002 API shape.

P2-GATE-002 fast tests must use synthetic sample arrays only. They must not
read:

- TouhouNewWorld `bin` or `resource` data;
- old backup runtime files;
- old report/status files;
- audio files or codec fixtures as runtime behavior inputs.

## P2-GATE-002 Compatibility

P2-GATE-002 may implement:

- `YuAudio` target;
- test backend device;
- audio value/handle/status types;
- bounded synthetic sample source registration;
- bounded voice table;
- start/stop voice behavior;
- deterministic caller-driven mix into caller-owned buffers;
- test sink counters and snapshots.

P2-GATE-002 must not implement:

- real audio device backend;
- audio callback thread or streaming thread;
- file/resource reads;
- codec parser or decoder;
- BGM/SE business IDs;
- script, scene, UI, gameplay, or Game Adapter behavior;
- report, profiler, capture, oracle, dashboard, or editor tool output.

## Consequences

YuEngine gains a deterministic audio foundation that can prove mixer behavior
without depending on OS audio or original-game assets.

The cost is that no real playback, streaming, decode, or audio scene exists yet.
That is intentional. A future real backend gate can focus on callback and device
lifecycle after the public test backend and mixer rules are proven.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment.

If accepted, ADR-0012 becomes the architecture input for P2-GATE-002 Audio Test
Backend And Mixer Sink.
