# ADR-0010: Input Replay And Action Snapshot Boundary

Status: Accepted
Owner: 八云紫
Reviewers: 红美铃, 博丽灵梦, 大妖精, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, ADR-0006, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

YuEngine needs an Input boundary before UI, scene/world, gameplay facades, script
service calls, and the TouhouNewWorld Game Adapter can consume user intent.
Input must not begin as title-menu behavior, hard-coded game actions, or a
window-message dump.

The first Input decision therefore separates deterministic engine input state
from OS device collection, UI focus routing, text input, controller backends, and
game-specific action semantics.

## Decision

YuEngine introduces `YuInput` as the owner of:

- input device identifiers;
- action identifiers;
- setup-time action binding descriptors;
- bounded synthetic input events;
- deterministic per-frame input snapshots;
- replay fixtures for fixed input frames;
- explicit result/status values for invalid input and capacity failures.

The first Input slice is a synthetic replay and action snapshot skeleton only. It
does not read OS messages, poll hardware devices, route UI focus, expose text or
IME input, or define TouhouNewWorld action names.

## Input Model

First-slice input is value-based and deterministic.

Initial concepts:

```text
InputDeviceId
InputActionId
InputControlId
InputEvent
InputFrame
InputSnapshot
ActionBinding
InputReplay
InputStatus
```

Names may change during implementation review, but the responsibilities must
stay equivalent.

Rules:

- device IDs, control IDs, and action IDs are stable numeric identifiers;
- action bindings are setup-time data;
- first-slice control names are not strings on the frame path;
- synthetic events are sorted by replay frame order, not by wall-clock time;
- applying the same replay frames produces byte-identical snapshot values;
- unknown device, control, or action IDs return explicit status values;
- no behavior is observed only through log text or report output.

## Action Boundary

Actions are engine-level intent slots, not game business names.

P1-GATE-007 may define fixed action IDs such as `ActionA` and `ActionB` in
tests. It must not define title-menu, save, camera, tutorial, player attack,
spell, UI navigation, or original script service action semantics.

Action state contains only first-slice primitives:

- pressed/released boolean state;
- changed-this-frame boolean state;
- optional axis value with a fixed signed integer normalized range.

Text input, composition, pointer capture, multi-touch gestures, controller
rumble, dead-zone policy, and rebinding UI are future gates.

## Binding Identity And Cardinality

First-slice bindings are setup-time mappings from controls to actions.

Rules:

- the binding identity key is `(InputDeviceId, InputControlId)`;
- one `(InputDeviceId, InputControlId)` maps to at most one `InputActionId`;
- recording a second binding for the same `(InputDeviceId, InputControlId)`
  returns explicit duplicate-binding status and does not mutate binding state;
- multiple controls may map to the same `InputActionId`;
- one control may not fan out to multiple actions in P1-GATE-007;
- same-action multi-control merge is deterministic: accepted events are applied
  in replay insertion order, and the last valid event for that action in the
  frame defines the final action value.

Future gates may add action groups, chorded bindings, device-specific priority,
rebinding, or richer merge policies only after the first-slice snapshot behavior
is accepted.

## Axis Representation

First-slice axis values are deterministic integer values, not floating-point
values.

Rules:

- axis events carry a signed integer normalized value in the inclusive range
  `[-32767, 32767]`;
- `-32767` represents full negative, `0` represents neutral, and `32767`
  represents full positive;
- values outside that range return explicit invalid-value status and are not
  inserted into replay storage;
- P1-GATE-007 does not accept float axis input, so NaN, Inf, and negative-zero
  float semantics are outside this slice;
- future platform gates may define backend float or hardware-range conversion
  into this integer range.

## Replay Boundary

Replay is a deterministic test harness for input frames.

Rules:

- replay buffers are bounded by fixed frame and event capacities;
- replay events are inserted during setup only and are stored in insertion order
  inside each frame;
- frame application does not grow containers or allocate;
- replay overflow returns explicit capacity status and does not mutate the
  accepted event sequence;
- event order inside a frame is deterministic: events are applied in accepted
  insertion order, and the last valid event for an action wins the final value
  for that frame;
- `changed_this_frame` is true when at least one valid accepted event for that
  action occurs in the frame, even if the final value equals the previous
  snapshot value after a press-then-release pair;
- invalid replay events are rejected before insertion where possible; if an
  invalid event reaches frame application, that event is skipped, returns an
  explicit status, increments the rejected-event signal, and does not mutate the
  snapshot value or accepted-event signal;
- reset clears frame state without releasing and reallocating replay storage.

Replay fixtures are not an oracle or capture system. They are module-owned
deterministic inputs for Input tests and future higher-layer fixtures.

## Platform Boundary

P1-GATE-007 does not connect to OS input.

Blocked:

- Win32 message pump integration;
- Raw Input, XInput, HID, DirectInput, SDL, or controller backends;
- keyboard layout, Unicode text, IME, clipboard, or cursor capture;
- window focus, UI focus tree, or gameplay camera ownership.

Future platform input gates may translate platform events into the `YuInput`
event model only after the synthetic boundary is accepted and tested.

## Diagnostics Boundary

Diagnostics may observe Input counters later. Diagnostics must not own Input
behavior.

Rules:

- Input results are explicit status values.
- Disabled diagnostics/logging does not change snapshots or replay results.
- Report/capture/oracle output is not an Input runtime API.

## Memory And Thread Boundary

Memory:

- action bindings, replay frames, and snapshots have explicit owner and capacity;
- frame application must not allocate or grow storage;
- if `YuMemory` exists, allocation/accounting signals use its vocabulary;
- if not integrated, the limitation is explicit and cannot be counted as zero.

Thread:

- P1-GATE-007 is single-threaded and caller-driven;
- no background input thread, callback thread, or async queue is introduced;
- future device polling or platform callback gates must define synchronization
  and deterministic shutdown separately.

## Evidence Boundary

TouhouNewWorld title, save, menu, scene, actor, camera, and tutorial controls are
future Game Adapter evidence. They are not inputs to P1-GATE-007.

P1-GATE-007 fast tests must use synthetic generic controls and actions only.
They must not read:

- TouhouNewWorld `bin` or `resource` data;
- old backup runtime files;
- old report/status JSON or markdown as runtime behavior fixtures.

Original control facts may become adapter acceptance fixtures only after Input,
UI, Scene/World, and Game Adapter gates define the owning interfaces.

## P1-GATE-007 Compatibility

P1-GATE-007 may implement:

- `YuInput` target;
- input ID value types;
- action binding values;
- bounded synthetic event and replay storage;
- deterministic frame snapshot apply/reset behavior;
- explicit Input status/result values;
- deterministic counters for tests.

P1-GATE-007 must not implement:

- OS device polling or message pump integration;
- real keyboard, mouse, gamepad, HID, SDL, XInput, or Raw Input backend;
- UI focus routing or widget navigation;
- text input, Unicode, IME, clipboard, or cursor capture;
- gameplay actions or TouhouNewWorld adapter behavior;
- script, scene, world, UI, audio, render, resource, or tool behavior;
- report, capture, oracle, or dashboard output.

## Consequences

YuEngine gains a deterministic input-state foundation that higher layers can
consume without depending on platform messages or game-specific action names.

The cost is that no real hardware input exists yet. That is intentional. Real
device collection, focus routing, UI navigation, and Game Adapter semantics need
separate gates after this boundary is proven.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment.

If accepted, ADR-0010 becomes the architecture input for P1-GATE-007 Input
Replay And Action Snapshot Skeleton.
