# P2-GATE-005: Platform Window Native Surface And Event Pump

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-004, ENG-096
Related decisions: ADR-0002, ADR-0005, ADR-0006
Source baseline: `8836774`
Pre-audit evidence: ENG-099A boundary/reference PASS with hard proposal
constraints, ENG-099B implementability PASS, and ENG-099C test-strategy PASS
with hard conditions.
Approved proposal: `e196087`
Approval evidence: ENG-100A boundary/reference PASS with hard implementation
conditions, ENG-100B implementability/file-layout PASS, and ENG-100C
test-policy/gate-cost PASS.

## Layer

L0-L1 Platform and host boundary.

This gate starts the real hardware-facing Platform path after the landed test
tier labels. It defines the window, native surface record, and event pump
contract needed before RHI can bind a swapchain surface.

The gate does not approve RHI device creation, D3D11, DXGI swapchains, RenderCore,
mesh rendering, UI policy, resource loading, game adapter behavior, screenshots,
or visible-render proof.

```text
headless Platform baseline
-> Platform window descriptor and lifecycle contract
-> opaque native surface value for later RHI binding
-> bounded event pump and typed Platform events
-> deterministic contract tests
-> later isolated Win32 hardware-smoke tests
```

## Current Reality

Current `YuPlatform` owns only:

- `HeadlessHost`;
- `FixedFrameClock`;
- `HostError`, `HostStatus`, and `HostRunResult`;
- `IHostRuntime` and `IFrameClock`;
- `PlatformPerformanceSignal`.

Current Platform tests cover only 3 deterministic tests under the `Platform`
label. There is no OS window, native surface handle, Win32 implementation,
message pump, resize/focus/close state, raw device event capture, or GPU adapter
discovery.

`windows-fast-gate` is currently unfiltered and discovers 646 deterministic
tests. `windows-hardware-smoke` exists, filters `HardwareSmoke`, and currently
discovers 0 tests.

## Owns

This gate owns the first proposal for:

- a Platform-owned window descriptor;
- explicit Platform window create, destroy, show, hide, close, and poll
  lifecycle states;
- an opaque native surface value that can carry platform handle identity without
  exposing Windows SDK types in public headers;
- a bounded event pump step that writes events into caller-owned buffers;
- typed Platform events for close, resize, focus, minimized/restored, and raw
  keyboard/mouse capture where the platform can provide them;
- explicit status values for invalid descriptors, null pointers, unsupported
  platform backend, closed windows, capacity overflow, and lifecycle misuse;
- a snapshot value that exposes current lifecycle state, client extent, focus,
  close request, minimized/restored state, last status, and event-drop counters;
- Windows implementation planning behind the Platform boundary;
- deterministic tests for public contract, state transitions, bounded storage,
  and dependency boundaries.

## Does Not Own

This gate does not own:

- D3D11, DXGI, GPU adapter selection, device/context creation, swapchain,
  backbuffer, present, capture, shader, buffer, texture, pipeline, or mesh
  behavior;
- RHI factory/device changes beyond future consumption of an opaque native
  surface record;
- RenderCore passes, frame graph, draw scheduling, material system, or scene
  traversal;
- YuInput action mapping, focus policy above raw Platform events, UI navigation,
  text input, IME, clipboard, cursor policy, or gameplay input semantics;
- Resource, Package, File streaming, async IO, upload queues, or asset decode;
- World, Scene, Game Adapter, tools, reports, dashboards, screenshots, visual
  demo output, or manual window inspection;
- turning zero or skipped hardware-smoke discovery into proof that a backend
  works.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Platform/Application layer owns process windowing, OS messages, raw platform
  events, and native handle identity;
- RHI owns device, swapchain, present, capture, and resource submission;
- RenderCore owns render passes and frame scheduling above RHI;
- Input owns semantic device mapping and focus-aware snapshots above raw
  Platform events.

YuEngine must not copy UE or Unity source, public API names, private layout, or
module naming. This gate uses those engines only to keep responsibilities
separate.

## Dependencies

Allowed dependencies:

- `YuDiagnostics`;
- `YuMemory`;
- C++ standard library types already accepted by Platform;
- Windows APIs in Windows-only `.cpp` files or private internal headers;
- `user32` when a Windows source is added;
- CMake/CTest for source registration and deterministic Platform tests.

Forbidden dependencies:

- `YuRHI`, RenderCore, D3D11, DXGI, shader/pipeline/resource code;
- `YuInput` semantic mapping or action snapshots;
- `YuKernel` lifecycle changes beyond existing consumers continuing to build;
- `YuResource`, `YuPackage`, `YuFile` streaming behavior, World, UI, tools,
  report/evidence modules, or Game Adapter;
- Windows SDK types such as `HWND`, `HINSTANCE`, `WPARAM`, `LPARAM`, or
  `LRESULT` in public Platform headers.

## Public Surface Shape

The first slice should introduce small Platform-owned value types. Suggested
names:

- `PlatformWindowStatus`;
- `PlatformWindowDesc`;
- `PlatformNativeSurface`;
- `PlatformWindowEventType`;
- `PlatformWindowEvent`;
- `PlatformWindowPollResult`;
- `PlatformWindowSnapshot`;
- `WindowsPlatformWindow` or `PlatformWindowWindows`.

Public headers should carry opaque handle values as `std::uintptr_t` or an
equivalent typed value owned by Platform. They must not include `Windows.h`.

`PollEvents` should take a caller-owned `PlatformWindowEvent *` buffer and a
capacity, then return an explicit status and count. It should not return a
newly allocated container on the pump path.

If a Windows window procedure needs staging storage, that storage must be fixed
capacity, observable through snapshot/drop counters, and unable to hide overflow
or lost OS events.

## Lifecycle

The intended lifecycle is:

1. Caller builds a `PlatformWindowDesc`.
2. Platform validates descriptor values.
3. Platform creates or rejects the window with an explicit status.
4. Caller may show, hide, poll, query snapshot, and request close.
5. `PollEvents` drains a bounded amount of queued OS/platform events into a
   caller-owned buffer.
6. Resize, focus, close, minimized/restored, and raw device events are reported
   as typed Platform events.
7. Destroy is explicit and idempotent.
8. After destroy, native surface values are invalidated and later operations
   return explicit closed or invalid-lifecycle statuses.

Failure behavior:

- invalid descriptors fail without mutating existing window state;
- null output buffers fail before writing;
- zero-capacity event buffers report whether events remain without overrun;
- queue overflow is reported through status/snapshot counters;
- unsupported platform backends report `Unsupported` instead of creating
  placeholder handles;
- close requests are observable and do not silently destroy the window.

## Inputs

- window descriptor values such as title length limit, client width, client
  height, visibility flag, and optional fixed event queue capacity;
- platform message state from the OS backend;
- caller-owned event output buffer;
- existing Platform memory/performance accounting hooks where they are already
  available.

## Outputs

- explicit Platform window status;
- native surface value for later RHI binding;
- typed event records;
- poll result with count and status;
- snapshot values for lifecycle state, client size, focus, close request,
  minimized/restored state, and bounded event/drop counters;
- no screenshot, log, report, or visual demo output as proof.

## Test And Preset Strategy

The first slice chooses the conservative test route from ENG-099C:

- add deterministic Platform contract tests only;
- label those tests `Fast`, `ModuleFixture`, and `Platform`;
- add `PerformanceSmoke` only to allocation/storage-cost tests;
- add `EvidenceOracle` only to boundary/proof tests;
- keep `windows-hardware-smoke` at 0 tests for this first slice;
- do not register real Win32 `HardwareSmoke` tests until hardware-smoke
  isolation is approved in a later gate or amendment.

Reason:

- current `windows-fast-gate` is unfiltered;
- directly registering real `HardwareSmoke` tests would make default fast gate
  run real OS/window tests unless a separate isolation change is approved;
- P2-GATE-004 requires default deterministic evidence to remain stable.

Later real Win32 smoke tests may be proposed as `HardwareSmoke`, `Platform`, and
`Win32`, but only after the default fast gate is explicitly protected from
running them. Zero or skipped hardware smoke is never backend proof.

Expected first-slice test growth is about 8 to 12 deterministic Platform tests.
If no other tests land in the same slice, `windows-fast-gate` discovery should
move from 646 to roughly 654-658.

## Performance Constraints

Required deterministic signals:

- descriptor validation is bounded and allocation-free after setup;
- event pump writes to caller-owned buffers;
- pump path does not grow storage;
- internal event staging is fixed capacity if required;
- overflow is visible and does not mutate output past capacity;
- native surface value is a POD-style record;
- public headers do not pull in Windows SDK or upper-layer dependencies.

Blocking conditions:

- unbounded dynamic allocation on poll/update path;
- `std::vector` or heap-owned output returned by `PollEvents`;
- silent event loss without status or snapshot counters;
- public Platform headers exposing Windows SDK types;
- RHI, RenderCore, Input mapping, World, Resource, Package, UI, report, or Game
  Adapter dependencies;
- real hardware smoke entering default `windows-fast-gate`;
- screenshot/manual visual proof replacing public-interface assertions.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate --output-on-failure`
- `ctest --preset windows-fast-gate -N -L Platform`
- `ctest --preset windows-fast-gate -N -L Fast`
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`
- `ctest --preset windows-hardware-smoke -N`
- `git diff --check`

The implementation handoff must record:

- total deterministic test discovery before and after;
- `Platform`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` discovery
  counts;
- whether `HardwareSmoke` remains 0;
- whether `windows-fast-gate` remains deterministic and no-real-device;
- proof that public Platform headers do not include Windows SDK types or
  upper-layer module headers.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowStatus.h
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowDesc.h
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformNativeSurface.h
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowEventType.h
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowEvent.h
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowPollResult.h
Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowSnapshot.h
Src/YuEngine/Platform/Include/YuEngine/Platform/WindowsPlatformWindow.h
Src/YuEngine/Platform/Src/WindowsPlatformWindow.cpp
Tests/Platform/PlatformTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_005_PLATFORM_WINDOW_NATIVE_SURFACE_EVENT_PUMP.md
```

`CMakePresets.json` may be changed only if review explicitly approves a
hardware-smoke isolation amendment. The default first slice should not need that
change because it must not register real `HardwareSmoke` tests.

The first slice must keep `HeadlessHost` behavior stable and additive.

## Non-Goals

- No D3D11 implementation.
- No DXGI swapchain.
- No present/capture.
- No rendered triangle or mesh.
- No shader, texture, buffer, or pipeline state.
- No RenderCore.
- No Resource or Package loading.
- No real input semantic mapping.
- No UI or gameplay focus policy.
- No report, screenshot, or visual oracle.
- No Game Adapter.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the Platform boundary remains L0-L1 and does not absorb RHI,
  RenderCore, Input, UI, Resource, World, report, or Game Adapter ownership;
- 博丽灵梦 confirms the proposed public surface and file layout are locally
  implementable without Windows header pollution or unbounded pump allocation;
- 雾雨魔理沙 confirms the deterministic test plan protects `windows-fast-gate`
  and does not require real hardware/window smoke by default;
- 八云紫 confirms this gate remains before P2-GATE-006 and does not authorize
  D3D11 or rendering behavior.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.

## Approval Decision

P2-GATE-005 is approved for first slice after ENG-100 review closure.

Hard implementation conditions:

- Public Platform headers must not include `Windows.h`.
- Public Platform headers must not expose `HWND`, `HINSTANCE`, `WPARAM`,
  `LPARAM`, `LRESULT`, or upper-layer module headers.
- `PlatformNativeSurface` must remain an opaque Platform-owned value for later
  RHI consumption. It is not a RHI device, factory, swapchain, or present
  contract.
- `PollEvents` must write to caller-owned buffers and must not return
  `std::vector` or heap-owned output on the pump path.
- Event staging must be bounded. Overflow or loss must be visible through status
  and snapshot/drop counters.
- `HeadlessHost` behavior must remain stable and additive.
- The first slice may not add `YuRHI`, D3D11, DXGI, RenderCore, YuInput
  semantic mapping, UI, World, Resource, Package, report, visual proof, or Game
  Adapter dependencies.
- Raw keyboard/mouse capture remains typed Platform event data only.
- Windows implementation details must stay in Windows-only `.cpp` files or
  private internals. The first Windows dependency is limited to `user32` unless
  a later approval expands it.
- The first slice keeps `HardwareSmoke` at 0 and should not change
  `CMakePresets.json` unless a separate hardware-smoke isolation amendment is
  approved.
- New deterministic tests must be labeled `Fast`, `ModuleFixture`, and
  `Platform`; use `PerformanceSmoke` only for allocation/storage-cost tests and
  `EvidenceOracle` only for boundary/proof tests.
- Visual window presence, screenshots, logs, reports, skipped hardware smoke, or
  zero hardware smoke are not accepted as backend proof.
- Implementation handoff must record total deterministic discovery before and
  after, `Platform` / `Fast` / `PerformanceSmoke` / `EvidenceOracle` discovery
  counts, whether `HardwareSmoke` remains 0, and proof that public Platform
  headers do not include Windows SDK or upper-layer headers.

## First-Slice Result

The first slice landed at `e3e2ad7`.

Result:

- Added Platform window public API headers for status, descriptor, native
  surface, event type, event record, poll result, snapshot, and
  `WindowsPlatformWindow`.
- Added `WindowsPlatformWindow.cpp` with the Win32 implementation details kept
  out of public headers.
- Updated `CMakeLists.txt` to compile the Windows source and private-link
  `user32` on Windows.
- Added 10 deterministic Platform contract tests.
- `CMakePresets.json` was not changed.
- No `HardwareSmoke` tests were registered.

Evidence:

- `cmake --preset windows-fast-gate`: PASS.
- `cmake --build --preset windows-fast-gate`: PASS.
- `ctest --preset windows-fast-gate -N`: 656 tests, up from 646.
- Full `windows-fast-gate` passed at `656/656`.
- Label discovery: `Fast` 656, `ModuleFixture` 656, `Platform` 13,
  `PerformanceSmoke` 39, `EvidenceOracle` 80, and `HardwareSmoke` 0.
- `windows-hardware-smoke` remains 0 tests and exits successfully.
- Public Platform headers do not include `Windows.h` or expose `HWND`,
  `HINSTANCE`, `WPARAM`, `LPARAM`, or `LRESULT`.
- Win32 SDK symbols are confined to `WindowsPlatformWindow.cpp`.
- No RHI, D3D11, DXGI, RenderCore, YuInput semantic mapping, UI, World,
  Resource, Package, report, visual proof, or Game Adapter dependency was added.
