# P3-GATE-002: Serialization Value Stream

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Depends on: ADR-0015
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014
Source baseline: Phase 3 through `4373230`

## Layer

L3-L5 core runtime serialization boundary.

This gate proves deterministic, bounded, memory-backed value-stream behavior
without File IO, package parsing, Resource mutation, object construction,
reflection, script values, scene/world persistence, UI, gameplay, tools,
reports, or Game Adapter behavior.

## Pre-Approval Blockers

P3-GATE-002 must not be approved for implementation until:

- ADR-0015 is accepted;
- PM confirms sequencing against active Phase 1, Phase 2, and Phase 3 review
  pressure;
- performance review accepts the caller-provided-buffer/no-hot-path-allocation
  contract.

These blockers do not prevent architecture review. They prevent
`APPROVED_FOR_FIRST_SLICE`.

Current gate state: engine-reference, performance, and implementability/test
coverage review lanes have accepted the `10abe41` proposal, and ADR-0015 is
accepted. Evidence review is not triggered while original-save/resource/package
facts remain excluded. Task #69 closed PM/final sequencing and approves only
the bounded caller-provided-buffer value-stream first slice described here.

## Owns

This gate owns the first `YuSerialize` implementation slice for:

- stream header write/read;
- stream major/minor version validation;
- caller-provided byte-buffer writer and reader cursors;
- record and field ID vocabulary;
- primitive type tag vocabulary for signed/unsigned 32-bit and 64-bit integers
  plus fixed byte spans;
- unknown field skipping when the encoded field length is valid;
- explicit statuses, snapshots, and counters;
- disabled diagnostics equivalence.

## Does Not Own

This gate does not own:

- File IO, package parsing, or original resource/package reads;
- Resource handles, Resource mutation, decoders, cache, hot reload, or upload
  scheduling;
- object construction, object lookup, object handle serialization, or lifetime
  registry mutation;
- reflection metadata, serialization attributes, generated code, or schema
  compiler;
- strings, string tables, compression, encryption, checksums, save slots, or
  cloud save;
- script/native bridge values or VM state;
- scene/world/component/transform persistence;
- UI, gameplay, audio, render, input, physics, animation, or Game Adapter
  behavior;
- reports, capture files, oracle output, profiler traces, or tools.

## Bounds

The first slice uses these bounds:

- 4096-byte caller-provided stream buffer;
- 32 records per stream;
- 64 fields per stream;
- 16 fields per record;
- 256-byte maximum fixed byte-span payload per field;
- 32-byte stream snapshot structure;
- 8 status counters.

Any implementation must return explicit overflow/truncation status before
writing or reading beyond these bounds. Failed write operations must not mutate
the caller-visible buffer range beyond bytes already committed by prior
successful operations.

## Status Vocabulary

The first slice must expose explicit statuses equivalent to:

- `Ok`
- `BufferTooSmall`
- `RecordCapacityExceeded`
- `FieldCapacityExceeded`
- `FieldPayloadTooLarge`
- `InvalidHeader`
- `UnsupportedVersion`
- `TruncatedStream`
- `UnknownTypeTag`
- `TypeMismatch`
- `DuplicateField`
- `MalformedFieldLength`

Exact names may differ, but tests must assert equivalent behavior and
no-mutation requirements.

Diagnostics availability is not part of the serialization write/read result
vocabulary. Disabled or unavailable diagnostics may be observed only through the
diagnostics lane itself; it must not change serialization statuses, committed
buffer bytes, reader cursor state, writer cursor state, or snapshot counters.

## Required Fast Tests

The first implementation slice must add fast tests equivalent to:

- `Serialize_WriteReadPrimitives_RoundTripsDeterministically`
- `Serialize_StreamHeader_RejectsInvalidMagicOrVersion`
- `Serialize_WriterBufferOverflow_ReturnsStatusWithoutOverrun`
- `Serialize_RecordCapacityOverflow_DoesNotMutate`
- `Serialize_FieldCapacityOverflow_DoesNotMutate`
- `Serialize_FixedBytesPayloadLimit_ReturnsExplicitStatus`
- `Serialize_ReaderRejectsTruncatedStream`
- `Serialize_ReaderRejectsMalformedFieldLength`
- `Serialize_ReaderRejectsUnknownTypeTag`
- `Serialize_ReaderTypeMismatch_ReturnsExplicitStatus`
- `Serialize_DuplicateField_ReturnsExplicitStatus`
- `Serialize_UnknownFieldWithValidLength_CanSkipDeterministically`
- `Serialize_DisabledDiagnostics_DoesNotChangeResults`
- `Serialize_NoFilePackageResourceObjectOrGameAdapterDependency`
- `Serialize_NoHiddenAllocationInReadWritePath`
- `Serialize_SnapshotReportsCountsAndLastStatus`

## Determinism Requirements

For the same write sequence and buffer capacity, the produced bytes must be
identical across repeated runs on the supported Windows fast gate preset.

The first slice fixes little-endian encoding. It must not use host pointer
values, addresses, locale, wall-clock time, thread ID, filesystem state, or
iteration over unordered containers to determine output bytes.

## Diagnostics And Accounting

Diagnostics are observational only. Serialization result behavior is defined by
return statuses and buffer contents, not logs, reports, traces, or oracle files.
Writer and reader APIs must not return a diagnostics-unavailable status.

If memory/accounting signals are exposed, they must use explicit
tracked-path-only wording from accepted `YuMemory` behavior. This gate must not
claim CRT/STL/general heap zero coverage.

## Evidence And Original-Game Inputs

No original-game evidence is needed for this first slice. Fast tests must use
synthetic buffers built in test code.

The first slice must not read TouhouNewWorld save files, packages, resources,
scripts, scenes, old backup runtime files, old reports, or Game Adapter data.

## Review Exit Criteria

This gate can move to `APPROVED_FOR_FIRST_SLICE` only when:

- ADR-0015 is accepted;
- engine-reference review accepts the boundary placement;
- performance review accepts the bounded caller-provided-buffer cost model;
- implementability/test review accepts the required tests and public surface;
- evidence-boundary concerns are either not triggered or accepted;
- PM confirms sequencing and implementation ownership.
