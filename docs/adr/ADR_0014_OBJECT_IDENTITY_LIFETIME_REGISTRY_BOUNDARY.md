# ADR-0014: Object Identity And Lifetime Registry Boundary

Status: Proposed
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0009, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

YuEngine needs engine-owned object identity before script, scene/world, UI,
animation, physics, and Game Adapter code can reference runtime instances.
Without a narrow object boundary, upper systems will be tempted to use resource
handles, raw pointers, strings, or original-game service state as object
identity.

The first object decision must therefore define a small object identity and
lifetime registry. It must not become an entity/component system, a scene graph,
a reflection system, a script object model, or a resurrection of old
`FrameRuntime.cpp` business state.

## Decision

YuEngine introduces `YuObject` as the owner of:

- runtime object identity vocabulary;
- object type identifiers;
- generation-checked object handles;
- bounded synthetic object registry behavior;
- explicit acquire/release reference counting for object handles;
- explicit destroy and stale-handle semantics;
- object registry snapshots and counters for tests;
- explicit object status values.

The first object slice is an identity and lifetime registry only. It does not
own components, transforms, scene hierarchy, update order, serialization payloads,
script binding, Resource lifetime, UI widgets, gameplay, tools, reports, or
Game Adapter behavior.

## Object Identity

An object handle is not a raw pointer, resource handle, script handle, scene
node, or string name.

First-slice identity contains:

- `ObjectTypeId`;
- stable registry slot/index;
- handle generation;
- explicit object lifecycle state.

Rules:

- object handles compare by slot/index and generation;
- object type identifiers are fixed numeric values in first-slice tests;
- no string lookup is required for create, acquire, release, validate, or
  destroy;
- optional debug labels, if any, are setup/test metadata and are not identity;
- destroying an object increments generation before slot reuse so stale handles
  cannot validate or acquire a new object;
- handle validation returns explicit status and never falls back to log text.

## Lifetime Boundary

First-slice object states:

- invalid;
- alive;
- referenced;
- destroyed;
- stale generation.

Rules:

- creating with an invalid object type fails explicitly and does not mutate
  registry state;
- creating beyond capacity fails explicitly and does not mutate registry state;
- validating, acquiring, releasing, or destroying an invalid, stale-generation,
  or already-destroyed handle fails explicitly and does not mutate lifecycle or
  reference counters;
- repeated acquire of the same valid handle increments a `uint32_t` reference
  count and returns success;
- acquiring when reference count is already `UINT32_MAX` fails explicitly and
  does not change reference count;
- releasing a handle that is not currently acquired fails explicitly and does
  not mutate reference count;
- destroying an object with outstanding references fails explicitly;
- destroying a valid unreferenced object succeeds, increments generation, and
  frees the slot for later reuse;
- reused slots must return handles with the new generation.

For deterministic overflow testing, the first slice may allow a setup-only
synthetic descriptor field to seed a test object with an initial reference count,
including `UINT32_MAX`. That fixture field is not a runtime API and must not be
usable after object creation.

There is no deferred-destroy queue, garbage collection, ownership graph, weak
reference table, or cross-thread lifetime policy in this first slice.

## Snapshot Boundary

Object snapshots are deterministic test observations. They are not reports or
runtime ownership APIs.

First-slice snapshots expose:

- registry capacity;
- type capacity;
- alive object count;
- referenced object count;
- destroyed object count;
- failed operation count;
- last explicit status;
- allocation/accounting status.

Snapshot fields must change only as a result of explicit object operations.
Failed operations that are specified as no-mutation failures may update the last
status and failed operation count, but must not change lifecycle or reference
counters.

## Relationship To Resource And World

`YuObject` is runtime-instance identity. `YuResource` is asset/resource identity.
The first object slice must not depend on Resource handles, Resource logical
keys, package facts, or File paths.

Blocked:

- treating file paths or resource logical keys as object identity;
- using Resource handles as object handles;
- automatic object creation from packages, resources, scripts, or scene files;
- component attachment;
- scene hierarchy;
- world update phases;
- render/audio/input/script/UI/gameplay ownership.

Future world and script gates may reference object handles only after this
boundary is accepted and implemented.

## Memory Boundary

Object registry storage is bounded.

If the P1-GATE-002 `YuMemory` implementation is accepted before the first object
implementation starts, object allocation/accounting signals use `YuMemory`
vocabulary. This does not claim CRT/STL/general heap coverage.

If the `YuMemory` implementation receives a blocking rewrite that removes the
needed vocabulary, P3-GATE-001 must be amended before implementation handoff.

## Thread Boundary

P3-GATE-001 is synchronous and single-threaded.

Blocked:

- cross-thread object mutation;
- lock-free registry;
- worker job ownership;
- async create/destroy;
- callbacks that execute object lifecycle work;
- deferred-destroy queues.

Future multi-threaded object lifetime requires accepted Thread/Task semantics
and a separate gate.

## Diagnostics Boundary

Diagnostics may observe object counters later. Diagnostics must not own object
behavior.

Rules:

- object results are explicit statuses;
- disabled diagnostics/logging does not change object results;
- tests observe statuses, snapshots, and counters, not logs or reports;
- report/capture/oracle output is not a `YuObject` runtime API.

## Evidence Boundary

TouhouNewWorld actors, scripts, save data, scene facts, object names, and old
backup runtime service state are future validation evidence only. They must not
define `YuObject` APIs or fast tests.

P3-GATE-001 fast tests use synthetic object descriptors only. They must not
read:

- `C:\Steam\steamapps\common\TouhouNewWorld\resource`;
- original script, scene, save, actor, camera, or tutorial evidence;
- old backup runtime files;
- old report/status JSON or markdown files.

Original object or actor facts may enter only after a Game Adapter or world
validation gate defines the owning interface and evidence fixture boundary.

## P3-GATE-001 Compatibility

P3-GATE-001 may implement:

- `YuObject` target;
- `YuObjectTests` target;
- object type and handle value types;
- bounded synthetic object registry;
- create, validate, acquire, release, destroy behavior;
- explicit object status/result values;
- deterministic snapshots/counters for tests.

P3-GATE-001 must not implement:

- component model;
- transform or scene hierarchy;
- world/scene lifecycle;
- script object binding or native bridge;
- serialization payload format;
- reflection/type metadata beyond fixed numeric type IDs;
- Resource registry mutation;
- package or File reads;
- render/audio/input/UI/gameplay behavior;
- reports, capture, oracle, dashboard, or tool output;
- Game Adapter behavior.

## Consequences

YuEngine gains a narrow object identity foundation that later script, world, UI,
animation, physics, and adapter systems can reference without raw pointers,
strings, or resource handles standing in for runtime object identity.

The cost is that no scene, component, reflection, or script behavior exists yet.
That is intentional. Those systems need separate gates after object identity is
accepted.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment.

If accepted, ADR-0014 becomes the architecture input for P3-GATE-001 Object
Identity And Lifetime Registry.
