# P2-GATE-010: Thread Worker And Async IO Substrate

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P1-GATE-003, P2-GATE-004, ENG-096, ENG-110, ADR-0007
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0007, ADR-0011
Source baseline: `f55f6dd`
Proposal commit: `2e7e6d6`
Approval evidence: ENG-111A boundary/quality PASS, ENG-111B implementability
PASS, and ENG-111C test-policy PASS.

## Layer

L1-L3 lower-engine substrate.

This gate proposes the first production worker and async file-completion
substrate after the landed RHI visible-triangle hardware proof. It is not a
renderer, mesh, Resource, Package, or upload-queue gate. The goal is to prove
bounded worker ownership and async file completion before later Resource,
Package streaming, upload, real audio, input, or RenderCore gates use
background work.

```text
existing bounded YuThread queue
-> one explicit worker lifecycle fixture
-> bounded submit and completion records
-> async File read completion over caller-owned request/result slots
-> deterministic shutdown, drain, cancel, and snapshot proof
-> later package/resource streaming and upload queue gates
```

## Current Reality

P1-GATE-003 landed the deterministic `YuThread` skeleton:

- `BoundedTaskQueue`;
- `InlineTaskExecutor`;
- task status, result, callback, shutdown policy, and scheduler snapshot;
- caller-thread drain only;
- no worker thread, wait primitive, worker ownership, or async IO.

P1 File work landed a deterministic loose-file fixture:

- virtual path normalization;
- mount table lookup;
- synchronous `LooseFileSource::Read`;
- file snapshots and bounded fixture reads;
- no async request queue, background read, completion queue, or streaming.

P2-GATE-009 landed the first real RHI visible geometry proof at `f55f6dd`.
That proves D3D11 draw and capture bytes, but it does not create the worker,
async IO, Resource loading, upload, mesh, material, or RenderCore substrate
needed by later hardware-engine slices.

## Owns

This gate owns the proposal for:

- a minimal `YuThread` worker lifecycle around existing bounded task records;
- explicit worker start, submit, request-stop, join, and shutdown status values;
- bounded work and completion accounting with no hidden unbounded queue;
- deterministic worker snapshots for submitted, started, completed, failed,
  canceled, rejected, pending, and max-depth counts;
- a first async file read substrate over the existing File module;
- caller-owned async file request and result slots;
- explicit async completion status for success, rejection, cancellation,
  read failure, and shutdown;
- tests that prove drain and cancel behavior without relying on logs, reports,
  sleeps, screenshots, or manual inspection;
- CTest labels that keep this deterministic lane in `windows-fast-gate` and
  expose `Thread`, `File`, `AsyncIO`, `Fast`, `PerformanceSmoke`, and
  `EvidenceOracle` discovery.

## Does Not Own

This gate does not own:

- Resource asset identity, dependency graph, cache ownership, load states, or
  lifetime policy;
- Package streaming, package parser behavior, archive IO, or manifest mutation;
- RHI upload queues, staging buffers, GPU fences, texture import, image decode,
  mesh upload, static mesh asset pipeline, material system, or shader tooling;
- RenderCore, render graph, frame pass scheduling, render queues, or scene
  traversal;
- real audio callback threads, codecs, streaming audio, WASAPI, or XAudio2;
- OS input device bridge, UI navigation, script, World, scene policy, gameplay,
  reports, tools, profiling UI, or Game Adapter behavior;
- work stealing, fibers, coroutines, lock-free queues, thread pools, or priority
  scheduling beyond the existing deterministic queue policy.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- `YuThread` owns worker lifetime, bounded task submission, completion records,
  drain/cancel policy, and shutdown proof;
- `YuFile` owns path normalization, mount lookup, loose-file reads, and async
  file-read completion status;
- `YuResource` owns later asset identity, dependency lifetime, and cache policy;
- RHI owns GPU resources, upload, draw, present, capture, and backend state;
- RenderCore owns frame passes and render submission above RHI.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep worker, file, resource, render,
and gameplay boundaries separate.

## Dependencies

Allowed dependencies:

- existing `YuThread` public and private files;
- existing `YuFile` public and private files;
- existing `YuMemory` accounting vocabulary already used by Thread and File;
- C++ standard threading and synchronization primitives inside `YuThread`
  implementation files;
- `YuFile` private linkage to `YuThread` if the async file substrate is owned by
  File while worker details stay hidden from File public headers;
- test-only fixture files under `Tests/Thread` and `Tests/File`.

Forbidden dependencies:

- `YuResource`, `YuPackage`, `YuRHI`, `YuPlatform`, RenderCore, UI, Script,
  World, tools, reports, or Game Adapter from production `YuThread`;
- `YuResource`, `YuPackage`, `YuRHI`, RenderCore, UI, Script, World, tools,
  reports, or Game Adapter from the async File substrate;
- Windows SDK, IOCP, overlapped IO, platform file handles, or OS-specific file
  APIs in public Thread or File headers;
- Resource loading, package streaming, image decoding, texture upload, mesh
  loading, shader compilation, or render submission in tests or runtime code;
- log text, report files, or timing sleeps as proof.

## Public Surface Shape

The first slice should keep public API expansion small and value-based.
Suggested additions:

- `ThreadWorkerDesc` with fixed queue capacity and explicit shutdown policy;
- `ThreadWorkerStatus` for explicit lifecycle and submission results;
- `ThreadWorkerSnapshot` for worker and queue counters;
- `ThreadWorker` or equivalent owner type with explicit initialize, submit,
  request-stop, join, shutdown, and snapshot operations;
- `AsyncFileReadRequest` with a mount, virtual path, and caller-owned output
  slot identifier;
- `AsyncFileReadResult` or completion record with explicit status, byte count,
  and request index;
- `AsyncFileReadQueue` or equivalent File-owned adapter with bounded pending and
  completion capacity.

Public headers must not expose platform thread handles, Windows file handles,
Resource, Package, RHI, RenderCore, UI, World, report, screenshot, visual proof,
or Game Adapter types.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller creates a worker descriptor with fixed work and completion capacity.
2. Caller initializes a worker with explicit ownership and no hidden global
   thread.
3. Caller starts the worker.
4. Caller submits fixed tasks or async file read requests.
5. Worker executes queued work and writes bounded completion records.
6. Caller drains completion records into caller-owned result storage.
7. Caller requests shutdown with drain or cancel policy.
8. Caller joins or explicitly shuts down the worker.
9. Snapshots prove no pending work remains and capacities did not grow.

Failure behavior:

- submit before start returns explicit status;
- submit after shutdown returns explicit rejection;
- queue capacity overflow returns explicit rejection without mutation outside
  the failed status/counter;
- completion capacity overflow returns explicit status and does not overrun
  caller-owned storage;
- file path rejection and file read failure are preserved as File statuses;
- cancel shutdown records canceled work and does not expose partial success;
- missing worker support is a blocker, not accepted proof.

## Inputs

- fixed worker queue and completion capacities;
- caller-owned task contexts;
- caller-owned async file read requests;
- existing mount table and loose-file fixture inputs;
- caller-owned completion/result storage.

## Outputs

- explicit Thread and File async status values;
- deterministic worker and queue snapshots;
- async file completion records with byte counts;
- no Resource handles, package records, GPU upload objects, reports, logs, or
  visual proof as runtime outputs.

## Test And Preset Strategy

Default `windows-fast-gate` remains deterministic:

- worker and async file tests are labeled `Fast`, `ModuleFixture`, `Thread`,
  `File`, and `AsyncIO` as appropriate;
- capacity and allocation checks are labeled `PerformanceSmoke`;
- dependency and proof-shape tests are labeled `EvidenceOracle`;
- no `HardwareSmoke`, `D3D11`, or `Win32` tests are required by this gate;
- `windows-hardware-smoke` is unaffected.

Expected deterministic first-slice growth is about 8 to 16 tests. The gate must
not remove or relabel existing deterministic tests to make the count look
smaller.

## Performance Constraints

Required signals:

- declared work queue capacity;
- declared completion capacity;
- submitted, started, completed, failed, canceled, rejected, pending, and
  max-depth counts;
- shutdown policy and result;
- allocation count or explicit setup-only allocation proof;
- no dynamic growth during the measured worker/async fixture;
- no blocking file IO inside render, audio callback, input, Resource, Package,
  or gameplay-owned code paths.

Blocking conditions:

- unbounded queue or completion storage;
- hidden global worker or static non-POD runtime object;
- public OS thread or file handle exposure;
- time-sleep based correctness;
- Resource, Package, RHI upload, RenderCore, mesh, material, scene, UI, World,
  report, or Game Adapter scope is required;
- async file completion depends on log output, report output, or manual
  inspection.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate --output-on-failure`
- `ctest --preset windows-fast-gate -N -L Thread`
- `ctest --preset windows-fast-gate -N -L File`
- `ctest --preset windows-fast-gate -N -L AsyncIO`
- `ctest --preset windows-fast-gate -N -L Fast`
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`
- `git diff --check`

The implementation handoff must record:

- deterministic discovery before and after;
- `Thread`, `File`, `AsyncIO`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`,
  and `HardwareSmoke` discovery counts;
- proof that default `windows-fast-gate` stays deterministic;
- worker lifecycle proof shape;
- async file completion proof shape;
- public-header dependency scan;
- production dependency scan for forbidden modules;
- regression evidence for existing Thread queue and File mount/read tests.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorker.h
Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerDesc.h
Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerSnapshot.h
Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerStatus.h
Src/YuEngine/Thread/Src/ThreadWorker.cpp
Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadQueue.h
Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadRequest.h
Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadResult.h
Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadStatus.h
Src/YuEngine/File/Src/AsyncFileReadQueue.cpp
Tests/Thread/ThreadTests.cpp
Tests/File/FileTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_010_THREAD_WORKER_AND_ASYNC_IO_SUBSTRATE.md
```

`CMakePresets.json` is not expected to change. Any CMake change must preserve
the default `windows-fast-gate` behavior.

## Non-Goals

- No Resource loading.
- No Package streaming.
- No File watcher.
- No OS-specific async file API.
- No IOCP or overlapped IO.
- No RHI upload queue.
- No texture import, image decode, mesh asset pipeline, or material system.
- No RenderCore.
- No real audio callback.
- No OS input bridge.
- No UI, World, gameplay, report, screenshot, visual oracle, or Game Adapter.
- No work stealing, fibers, coroutines, or general production thread pool.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the boundary keeps Thread/File async substrate separate from
  Resource, Package, RHI upload, RenderCore, mesh, material, scene, and Game
  Adapter ownership;
- 博丽灵梦 confirms the proposed file layout and worker/API additions are
  locally implementable without public OS handle exposure, unbounded queues, or
  hidden global worker lifetime;
- 雾雨魔理沙 confirms the test and label policy is enforceable in
  `windows-fast-gate` and does not require real hardware, timing sleeps, logs,
  reports, or manual proof;
- 八云紫 confirms this gate is the next approved lower-engine substrate step
  after P2-GATE-009 and before Resource/Package streaming, upload queue, real
  audio callback, OS input bridge, RenderCore, or static mesh implementation
  tasks.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.

## Approval Decision

P2-GATE-010 is approved for first slice after ENG-111 review closure.

Hard implementation conditions:

- The first slice remains lower-engine Thread/File async substrate work only.
- Public Thread/File headers may expose only value-based worker, async read,
  status, descriptor, snapshot, request, result, and caller-owned storage
  contracts.
- Public Thread/File headers must not expose platform thread handles, Windows
  SDK, IOCP, overlapped IO, OS file handles, Resource, Package, RHI, RenderCore,
  UI, World, report, screenshot, visual proof, or Game Adapter types.
- `std::thread`, `std::mutex`, `std::condition_variable`, and any wait primitive
  must remain private to `YuThread` implementation files. They must not appear
  in File public headers.
- File may use Thread only through private implementation linkage for the async
  adapter. File public headers must remain value-based and OS-handle-free.
- The worker must be explicitly owned by caller-created objects. Hidden global
  workers, static non-POD runtime objects, and implicit background services are
  not approved.
- Work and completion storage must be bounded. Unbounded queues, dynamic growth
  in the measured fixture, and timing-sleep proof are blockers.
- Async file completion may wrap existing `MountTable` / `LooseFileSource`
  synchronous reads, but it must use caller-owned request/result slots and
  explicit completion statuses.
- Resource loading, Package streaming, RHI upload queues, texture import, image
  decode, mesh upload, static mesh, material system, RenderCore, render
  submission, real audio callback, OS input bridge, UI, World, reports, and Game
  Adapter behavior are not approved.
- `CMakePresets.json` is not expected to change. CMake changes must preserve the
  default deterministic `windows-fast-gate` behavior and keep
  `windows-hardware-smoke` unaffected.
- Proof must use bounded snapshots, counters, statuses, caller-owned storage,
  drain/cancel behavior, and dependency scans. Proof must not rely on sleeps,
  logs, reports, screenshots, manual inspection, or real hardware.
- The implementation handoff must record before/after deterministic discovery,
  label counts for Thread, File, AsyncIO, Fast, PerformanceSmoke,
  EvidenceOracle, and HardwareSmoke, public-header dependency scans, production
  dependency scans, and regression evidence for existing Thread queue and File
  mount/read tests.
- Allowed implementation paths must use exact repository casing:
  `Src/YuEngine/Thread/...`, `Src/YuEngine/File/...`, and `Tests/...`.
