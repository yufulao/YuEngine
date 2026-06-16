# P2-GATE-012: Platform Input Device Bridge

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NOT_APPROVED`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P1-GATE-007, P2-GATE-004, P2-GATE-005, ENG-096
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0010
Source baseline: `1d7d2ca`

## Layer

L1-L3 lower-engine input backend proof.

This gate proposes the first platform input device bridge after the landed
Platform window/event pump, deterministic Input replay/action snapshot, and
hardware-smoke test tiering. It is not a UI navigation, menu, gameplay action,
Script, World, original-game control, report, or Game Adapter gate. The goal is
to prove that OS input events can be translated into bounded `YuInput` values
without leaking platform handles or UI/game semantics into the public input
boundary.

```text
existing Platform window/event pump
-> private Win32 input bridge adapter
-> backend-neutral input device/control events
-> focus-aware bridge snapshot
-> deterministic replay/action binding remains unchanged
-> optional hardware-smoke input bridge proof isolated from default fast gate
-> later UI navigation, gameplay mapping, remap UI, script, and Game Adapter
```

## Current Reality

P1-GATE-007 landed deterministic `YuInput` replay/action snapshot behavior:

- value-based device IDs, control IDs, action IDs, events, and statuses;
- setup-time action bindings;
- bounded synthetic replay frames;
- deterministic per-frame action state snapshots;
- fixed integer axis values;
- deterministic `Input_` fast tests;
- no OS device polling, Win32 messages, Raw Input, XInput, DirectInput, HID,
  SDL, UI focus routing, text input, gameplay action names, Script, World,
  reports, or Game Adapter behavior.

P2-GATE-005 landed Platform window/native surface/event pump behavior, but it
explicitly did not authorize `YuInput` semantic mapping. P2-GATE-004 landed test
labels and `windows-hardware-smoke`. P2-GATE-011 landed real Audio callback
proof at `1d7d2ca`, leaving Input as the next lower-engine hardware bridge in
the current queue.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N -L Input`: `18`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-fast-gate -N -L Fast`: `704`.

## Owns

This gate owns the proposal for:

- explicit platform input backend kind and unavailable/unsupported statuses;
- value-based platform input device descriptor;
- value-based platform input event records for keyboard, mouse buttons, mouse
  movement, wheel, and optional gamepad unavailable status;
- focus-aware bridge snapshot counters;
- bounded event buffers and overflow counters;
- private Win32 bridge implementation that translates platform messages or
  bounded injected test events into `YuInput` values;
- deterministic tests for public value contracts, focus gating, capacity, and
  dependency boundaries;
- optional hardware-smoke test that proves bridge discovery or injected Win32
  message translation through counters/statuses, not manual input.

## Does Not Own

This gate does not own:

- UI navigation, widget focus, menu commands, title-screen input, gameplay
  action semantics, remap UI, Script services, World update order, scene
  behavior, or Game Adapter behavior;
- text input, Unicode character composition, IME, clipboard, cursor capture,
  raw mouse relative mode, multi-touch gestures, controller vibration, or
  platform accessibility policy;
- Raw Input, XInput, DirectInput, HID, SDL, or multi-backend device management
  unless implementation review narrows one specific first-slice path;
- report output, screenshot proof, manual key/mouse proof, log parsing, or
  visual inspection;
- making the default `windows-fast-gate` require a real keyboard, mouse,
  gamepad, desktop focus, or interactive window.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- `YuPlatform` owns window lifetime, native handles, and the platform event
  pump;
- `YuInput` owns input IDs, event values, action bindings, replay/snapshot
  contracts, and bridge counters;
- the private Windows bridge owns OS message translation and platform API calls;
- UI later owns widget focus/navigation semantics;
- Script, World, and Game Adapter later own gameplay action meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate only uses those engines to keep platform event collection,
input state, UI focus, and gameplay mapping separate.

## Dependencies

Allowed dependencies:

- existing `YuInput` public and private files;
- existing `YuPlatform` public value contracts for window/event-pump connection;
- private Windows implementation files for Win32 message translation;
- CMake/CTest labels and optional `windows-hardware-smoke` admission;
- test-only fixtures under `Tests/Input`.

Forbidden dependencies:

- UI, Script, World, Game Adapter, Resource, Package, RHI, Audio, RenderCore,
  reports, screenshots, visual proof, tools, or original-game evidence from
  production `YuInput`;
- Windows SDK types, HWND, HINSTANCE, HANDLE, RAWINPUT, XINPUT structures,
  DirectInput interfaces, HID handles, or callback objects in public `YuInput`
  headers;
- text/IME/clipboard/cursor-capture APIs in this first slice;
- proof that depends on logs, reports, screenshots, manual input, or visual
  output.

## Public Surface Shape

The first slice should keep public API expansion small and value-based.
Suggested additions:

- `InputBackendKind` with `Replay` and one explicit platform bridge value such
  as `Win32`;
- `InputDeviceKind` for keyboard, mouse, and optional gamepad-unavailable
  reporting;
- `InputBridgeDesc` with fixed event capacity, accepted backend, and focus
  policy values;
- `InputBridgeEvent` or equivalent backend-neutral event record;
- `InputBridgeSnapshot` with accepted, rejected, overflow, focus-lost,
  focus-gained, unavailable, and max-queued counters;
- an explicit `InputBridge` owner with initialize, attach/detach window or
  platform source, pump/translate, drain, shutdown, and snapshot behavior.

Public headers must not expose Windows headers, HWND, HINSTANCE, HANDLE,
RAWINPUT, XINPUT, DirectInput, HID, platform callback object, UI, Script, World,
Game Adapter, report, screenshot, visual proof, or original-game control types.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller creates a bridge descriptor with backend, event capacity, and focus
   policy.
2. Caller initializes an explicit input bridge owner.
3. Private implementation binds to a platform event source or injected test
   source.
4. Caller pumps or translates bounded platform input events.
5. Bridge writes backend-neutral input events into caller-owned or owner-owned
   bounded storage.
6. Caller forwards accepted bridge events into existing `InputReplay` or
   snapshot tests where appropriate.
7. Caller snapshots counters and statuses.
8. Caller drains and shuts down the bridge.

Failure behavior:

- unsupported backend returns explicit status;
- missing or invalid platform source returns explicit unavailable or invalid
  source status;
- focus-lost policy either rejects or records input with an explicit counter;
- event-buffer overflow returns explicit status and increments an overflow
  counter without growing storage;
- invalid key, mouse button, wheel, or axis values return explicit status
  without mutating accepted-event state;
- shutdown is idempotent.

## Inputs

- fixed bridge descriptor;
- platform focus state;
- bounded platform input messages or injected test messages;
- fixed keyboard and mouse control mapping table;
- optional real focused window for hardware-smoke proof.

## Outputs

- explicit input statuses;
- backend-neutral input events;
- input bridge snapshot counters;
- no UI commands, gameplay actions, world events, reports, screenshots, log
  tokens, or manual input proof.

## Test And Preset Strategy

Default `windows-fast-gate` remains deterministic and no-real-device:

- public value contract tests are labeled `Fast`, `ModuleFixture`, and `Input`;
- dependency and proof-shape tests are labeled `EvidenceOracle`;
- bounded queue and no-growth tests are labeled `PerformanceSmoke`;
- no `HardwareSmoke`, `Win32`, `RawInput`, `XInput`, or `DirectInput` labels are
  added to default deterministic tests.

Optional real-device or platform-message proof is isolated:

- add a separate `YuInputHardwareSmokeTests` target only if implementation
  review confirms a non-interactive local proof can be enforced;
- hardware tests are labeled `HardwareSmoke`, `Input`, `Win32`, and the accepted
  backend label such as `Win32Message` or `RawInput` if a narrower backend is
  approved;
- `windows-hardware-smoke` may discover and run the real bridge test;
- default `windows-fast-gate -N -L HardwareSmoke` must remain `0`;
- proof must use counters/statuses and bounded event injection or platform
  messages, not sleeps, logs, reports, screenshots, manual key presses, manual
  mouse movement, or visual output.

Expected deterministic first-slice growth is about 6 to 12 fast tests plus 0 to
2 optional hardware-smoke tests. The gate must not remove or relabel existing
deterministic `Input_` tests to make the count look smaller.

## Performance Constraints

Required signals:

- declared event capacity;
- accepted event count;
- rejected event count;
- overflow count;
- focus-lost and focus-gained counts;
- unavailable or unsupported backend count;
- max queued event count;
- last input status;
- no dynamic growth during bridge translation fixture;
- no UI/script/world/gameplay dispatch in the bridge path.

Blocking conditions:

- public OS handle or Windows SDK exposure;
- platform bridge proof requires manual key presses, manual mouse movement,
  screenshot, report, log parsing, or visual inspection;
- default fast gate depends on a real window focus, keyboard, mouse, or gamepad;
- bridge path dispatches UI, Script, World, gameplay, or Game Adapter behavior;
- bridge path allocates or grows storage during the measured fixture;
- unbounded event queue or hidden global input device owner.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate --output-on-failure`
- `ctest --preset windows-fast-gate -N -L Input`
- `ctest --preset windows-fast-gate -N -L Fast`
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`
- `ctest --preset windows-hardware-smoke -N`
- `git diff --check`

If a hardware-smoke input bridge test is admitted, the implementation handoff
must also record:

- `ctest --preset windows-hardware-smoke --output-on-failure`;
- real backend test discovery count;
- explicit bridge proof shape;
- unavailable-device or unavailable-focus behavior if the machine cannot provide
  the accepted platform source.

The implementation handoff must record:

- deterministic discovery before and after;
- `Input`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and `HardwareSmoke`
  discovery counts;
- proof that default `windows-fast-gate` stays deterministic;
- public-header dependency scan;
- production dependency scan for forbidden modules and platform types;
- bridge lifecycle proof shape;
- regression evidence for existing replay/action `Input_` tests.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Input/Include/YuEngine/Input/InputBackendKind.h
Src/YuEngine/Input/Include/YuEngine/Input/InputDeviceKind.h
Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeDesc.h
Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeEvent.h
Src/YuEngine/Input/Include/YuEngine/Input/InputBridgeSnapshot.h
Src/YuEngine/Input/Include/YuEngine/Input/InputBridge.h
Src/YuEngine/Input/Include/YuEngine/Input/InputStatus.h
Src/YuEngine/Input/Src/InputBridge.cpp
Src/YuEngine/Input/Src/InputBridgeWindows.cpp
Tests/Input/InputTests.cpp
Tests/Input/InputHardwareSmokeTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_012_PLATFORM_INPUT_DEVICE_BRIDGE.md
```

`CMakePresets.json` is not expected to change. Any CMake change must preserve
the default `windows-fast-gate` behavior.

Implementation review may narrow file names, backend labels, or platform source
shape, but it may not expand beyond `YuInput`, private Windows implementation,
`Tests/Input`, root CMake labels, and this gate/queue documentation without
returning `NEEDS_ARCHITECTURE`.

## Non-Goals

- No UI navigation.
- No menu or title-screen behavior.
- No gameplay action names.
- No script/native input service.
- No World or scene integration.
- No Game Adapter.
- No text input, Unicode, IME, clipboard, cursor capture, or raw relative mouse
  mode.
- No controller vibration.
- No default fast-gate real-device requirement.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the boundary keeps platform input ownership separate from UI,
  Script, World, Game Adapter, reports, and original-game control semantics;
- 博丽灵梦 confirms the proposed file layout, bridge lifecycle, and bounded
  event/counter model are locally implementable without public OS handle
  exposure, hidden global device lifetime, unbounded buffers, bridge-path
  allocation, or UI/game dispatch;
- 雾雨魔理沙 confirms the test and preset policy is enforceable, keeps default
  `windows-fast-gate` deterministic, isolates any real platform proof in
  `windows-hardware-smoke`, and does not rely on logs, sleeps, reports,
  screenshots, manual key/mouse input, or visual output;
- 八云紫 confirms this gate is the next lower-engine proposal after P2-GATE-011
  and before static mesh, Resource/Package streaming, RenderCore, UI, World, or
  Game Adapter tasks.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.
