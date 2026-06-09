# P1-GATE-007: Input Replay And Action Snapshot Skeleton

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 博丽灵梦, 大妖精, 雾雨魔理沙 when implementation exists
Depends on: ADR-0010
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0007
Source baseline: Phase 1 through `b6c5720`

## Layer

L1-L3 Input boundary.

This gate proves synthetic input replay and deterministic action snapshots
without OS device polling, UI focus routing, text input, or game-specific action
semantics.

## Owns

This gate owns the first `YuInput` implementation slice for:

- input device IDs;
- input control IDs;
- input action IDs;
- setup-time action bindings;
- bounded synthetic input events;
- bounded replay frames;
- deterministic per-frame input snapshots;
- explicit Input result/status values;
- deterministic counters for tests.

## Does Not Own

This gate does not own:

- OS input backend integration;
- Win32 message pump, Raw Input, XInput, DirectInput, HID, SDL, or gamepad
  backend behavior;
- keyboard layout, text input, Unicode, IME, clipboard, cursor capture, or
  multi-touch gestures;
- UI focus, widget navigation, menu commands, or title-screen input;
- gameplay action semantics;
- script/native input services;
- scene/world update order;
- report/capture/oracle/tool output;
- TouhouNewWorld adapter behavior.

## UE/Unity Analogue

UE5 references:

- `Runtime\InputCore` and enhanced input concepts as responsibility references.

Unity references:

- Input System action maps, controls, and event traces as workflow references.

YuEngine decision:

- Start with deterministic synthetic events and snapshots.
- Keep platform device collection separate from engine input state.
- Keep UI/game adapter action meaning out of the low-level Input API.

## Lifecycle

First-slice lifecycle:

1. Setup creates a bounded Input context.
2. Setup registers fixed action bindings.
3. Setup records bounded synthetic replay frames.
4. Frame application consumes one replay frame into an `InputSnapshot`.
5. Snapshot exposes action state, changed state, axis state, and counters.
6. Reset clears per-frame changed state without freeing replay storage.

Failure behavior:

- duplicate binding returns explicit duplicate status;
- unknown device, control, or action returns explicit not-found status;
- binding capacity overflow returns explicit capacity status and does not mutate
  binding state;
- event capacity overflow returns explicit capacity status and does not mutate
  accepted replay events;
- invalid axis values return explicit invalid-value status and do not mutate
  snapshot state;
- applying frames past the replay end returns explicit end-of-replay status;
- disabled diagnostics/logging does not change any Input result.

## Inputs

- fixed numeric device IDs;
- fixed numeric control IDs;
- fixed numeric action IDs;
- setup-time action bindings;
- synthetic replay frames;
- optional memory tracker if P1-GATE-002 implementation exists.

## Outputs

- input snapshots;
- action pressed/released state;
- action changed-this-frame state;
- bounded axis values;
- input result/status values;
- snapshot/replay counters;
- allocation/accounting status using `YuMemory` vocabulary when available;
- no log/report text as behavior transport.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` for accounting vocabulary/signal tests when available;
- `YuDiagnostics` only if implemented, and only for disabled-behavior
  observation.

Target dependency expectation:

```text
YuInput
  -> optional YuMemory for accounting vocabulary/signal tests
  -> optional YuDiagnostics for disabled-behavior observation
```

`YuInput` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`,
`YuResource`, RHI, audio, script, scene/world, UI, tools, reports, or
TouhouNewWorld evidence in this first slice.

## Performance Constraints

Required deterministic signals:

- device capacity;
- action capacity;
- binding capacity;
- replay frame capacity;
- event-per-frame capacity;
- accepted event count;
- rejected event count;
- snapshot apply count;
- changed action count;
- allocation/accounting status using `YuMemory` vocabulary or explicit
  deferral;
- disabled diagnostics/logging behavior equivalence.

First-slice bounds:

- device capacity: 4 devices maximum;
- action capacity: 32 actions maximum;
- binding capacity: 64 bindings maximum;
- replay frame capacity: 16 frames maximum;
- event capacity per frame: 32 events maximum;
- axis value range: fixed signed normalized range `[-1.0, 1.0]`;
- frame application and snapshot reset must not allocate or grow storage.

Pass/fail rule:

- exceeding device, action, binding, frame, or event bounds is an explicit
  failure;
- frame application changing replay storage capacity, allocating, or depending
  on diagnostics/report output is a gate failure unless this gate is amended.

Blocking conditions:

- string action lookup on the frame path;
- unbounded event list, action map, or replay buffer;
- OS device polling or platform callback introduced by this slice;
- UI focus or gameplay action semantics introduced by this slice;
- hidden allocation in measured frame apply/reset paths;
- diagnostics/log/report output required for behavior;
- tests that validate behavior by parsing logs or reports.

## Tests

Fast gate tests required before the slice can be considered complete:

- `Input_RegisterActionBinding_ReturnsStableActionId`
- `Input_RegisterDuplicateBinding_ReturnsExplicitStatus`
- `Input_BindingCapacityOverflow_DoesNotMutate`
- `Input_ReplayFrame_AppliesButtonPressAndRelease`
- `Input_ReplayFrame_AppliesAxisValue`
- `Input_InvalidAxisValue_ReturnsExplicitStatus`
- `Input_UnknownDeviceOrAction_ReturnsExplicitStatus`
- `Input_EventCapacityOverflow_DoesNotMutateReplay`
- `Input_FrameSnapshot_IsDeterministicAcrossReplay`
- `Input_ResetClearsChangedStateWithoutClearingPressedState`
- `Input_DisabledDiagnosticsDoesNotChangeResults`
- `Input_FrameApply_DoesNotGrowReplayStorage`
- `Input_NoPlatformUiOrGameAdapterDependency`

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
src/yuengine/input/include/yuengine/input/
src/yuengine/input/src/
tests/input/
```

It may update root `CMakeLists.txt` only to add `YuInput` and `YuInputTests`.

It may not create placeholder directories or targets for platform input
backends, UI, script, scene/world, audio, RHI, resource, tools, report, capture,
oracle, or Game Adapter work.

## Non-Goals

- No real keyboard, mouse, gamepad, HID, SDL, XInput, Raw Input, or OS backend.
- No UI focus routing.
- No text input, Unicode, IME, clipboard, or cursor capture.
- No gameplay action names.
- No input remapping UI.
- No script/native input service.
- No scene/world update integration.
- No report/oracle/capture behavior.

## Evidence Inputs

No original-game evidence is required for the first slice.

TouhouNewWorld title/menu/control facts remain future Game Adapter and UI/Input
acceptance evidence only. They must not be read by P1-GATE-007 fast tests.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0010 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements;
- 博丽灵梦 confirms the replay/snapshot cost model and no-allocation frame path;
- 大妖精 confirms the public surface is implementation-reviewable;
- 雾雨魔理沙 confirms the first-slice boundaries and tests are locally
  enforceable.

If those conditions are not met, return `NEEDS_ARCHITECTURE` or
`NEEDS_PERFORMANCE` with exact missing fields.
