# ADR-0015: Serialization Value Stream Boundary

Status: Proposed
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md

## Context

YuEngine needs a stable serialization boundary before script, scene/world,
tools, resource import, and Game Adapter work can exchange persisted or
versioned values. Without a narrow value-stream layer, upper systems will be
tempted to write ad hoc binary blobs, copy original save/package formats into
core APIs, or hide behavior behind reports and oracles.

The first serialization decision must therefore define a deterministic,
bounded, memory-backed value stream. It must not become a scene save format, a
reflection system, a script object bridge, a Resource decoder, a File/package
loader, or a TouhouNewWorld save adapter.

## Decision

YuEngine introduces `YuSerialize` as the owner of:

- serialization status vocabulary;
- deterministic value-stream header and version vocabulary;
- primitive field identifiers and type tags;
- caller-provided byte-buffer writer and reader behavior;
- bounded record and field traversal rules;
- explicit overflow, truncation, duplicate, unknown, and type-mismatch statuses;
- serialization snapshots and counters for tests.

The first serialization slice is a value-stream primitive only. It does not own
reflection metadata, object construction, scene/world persistence, script VM
state, Resource lifetime, File IO, package parsing, save-game semantics,
tooling, reports, or Game Adapter behavior.

## Stream Shape

The first stream is an engine-owned synthetic format with:

- fixed magic value;
- format major/minor version;
- stream flags reserved for future compatibility;
- record count;
- record entries in declaration order;
- field entries in declaration order inside each record;
- little-endian numeric encoding.

The first slice may define numeric type tags for:

- unsigned 32-bit integer;
- signed 32-bit integer;
- unsigned 64-bit integer;
- signed 64-bit integer;
- fixed byte span.

It must not define floating-point, string, object handle, resource handle,
package entry, script value, scene node, transform, component, or gameplay
payload tags in the first slice.

## IDs And Versioning

`SerializationRecordId` and `SerializationFieldId` are fixed-width numeric
values. They are synthetic engine IDs for the first slice, not reflection names,
script symbols, original-game identifiers, or report keys.

Reader behavior must be deterministic:

- unsupported major version returns explicit status and does not read values;
- newer minor version may be read only if the required header and field encoding
  are supported;
- unknown record or field IDs can be skipped only when their length is valid and
  within the stream bounds;
- duplicate field IDs inside one record return explicit duplicate status unless
  the owning future gate defines repeated-field semantics.

## Memory And Allocation Policy

The first slice uses caller-provided byte buffers. Writer and reader objects
must not allocate heap storage in write/read hot paths. They may keep fixed-size
cursor and counter state only.

Any future dynamic schema registry, reflection table, string table, compression
dictionary, or save-file staging buffer requires a separate ADR/gate.

If a test needs accounting vocabulary, it must use explicit `YuMemory`
tracked-path wording only. It must not claim CRT/STL/general heap coverage.

## Diagnostics

Serialization behavior is validated by return statuses, snapshots, and counters.
Diagnostics may observe status and counts only.

`YuSerialize` must not depend on reports, JSON, trace capture, profiler output,
or oracle files. A disabled diagnostics channel must not change write/read
results.

Diagnostics availability is not a serialization status. Writer and reader APIs
must not return diagnostics-unavailable results or change committed buffer
bytes, cursor state, statuses, snapshots, or counters because diagnostics are
disabled or unavailable.

## Evidence Boundary

No original-game evidence is required for this first slice.

The first slice must not read or model:

- TouhouNewWorld save files;
- original package/resource files;
- old `FrameRuntime.cpp` serialization behavior;
- old reports/status schemas;
- script, scene, actor, camera, tutorial, or UI facts;
- Game Adapter behavior.

Those inputs remain future validation material only. They must not define
`YuSerialize` API shape, field IDs, stream tags, or fast-test fixtures.

## P3-GATE-002 Compatibility

P3-GATE-002 may implement:

- `YuSerialize` and `YuSerializeTests`;
- fixed stream header read/write;
- bounded writer over caller-provided byte buffer;
- bounded reader over caller-provided immutable byte buffer;
- primitive field write/read for the first type tags;
- unknown field skipping with explicit bounds checks;
- explicit status and snapshot/counter reporting;
- disabled diagnostics equivalence tests.

P3-GATE-002 must not implement:

- File IO or package parsing;
- Resource mutation, decoding, loading, or caching;
- object construction, object handle serialization, or registry lookup;
- reflection, serialization attributes, schema compiler, or generated code;
- string table, compression, encryption, checksum, save slots, or cloud save;
- script/native bridge values;
- scene/world/component persistence;
- UI/gameplay/Game Adapter behavior;
- reports, capture, oracle, or tooling output.

## Consequences

Upper systems get a deterministic value stream they can target later, but no
upper system can use this ADR to claim scene save, script persistence, Resource
serialization, package IO, or original-save compatibility.

The stream format is intentionally synthetic and small. Future compatibility
with real save data or tools must be proposed as separate ADR/gate work after
the primitive stream has been proven.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment before P3-GATE-002 can be approved.

If accepted, ADR-0015 becomes the architecture input for P3-GATE-002
Serialization Value Stream.
