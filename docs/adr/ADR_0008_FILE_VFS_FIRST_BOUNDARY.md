# ADR-0008: File And VFS First Boundary

Status: Proposed
Owner: 八云紫
Reviewers: 红美铃, 射命丸文, 博丽灵梦, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, ADR-0006, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_GATES.md

## Context

YuEngine needs a file and VFS boundary before resource handles, package formats, asset dependency graphs, upload scheduling, script loading, or original-game resource validation can appear. The Phase 0 reference map separates VFS/package lookup from Resource lifetime and upload scheduling. It also treats TouhouNewWorld resource facts as validation inputs, not lower-layer API drivers.

The original game has large `.pak` and `.dat` packages plus loose JSON/shader files under `resource/`. These facts prove YuEngine will eventually need robust package and loose-file handling, but they must not shape the first lower-layer API. The first File/VFS boundary must prove generic path and loose fixture behavior only.

Failure modes to avoid:

- original resource package details defining L1-L3 APIs;
- resource meaning hidden inside file lookup;
- async IO appearing before Thread/Task gates prove bounded queues and shutdown;
- path normalization allocating or parsing repeatedly in hot paths without a setup boundary;
- tests passing because an original file exists rather than because the file module behavior is correct.

## Decision

YuEngine introduces a `YuFile` module as the owner of path value types, path normalization, mount lookup, small loose-file read fixtures, and file error semantics.

P1-GATE-005 does not introduce resource handles, package parsing, async IO, cache policy, import/export tools, or original-game adapters. It introduces the smallest file boundary needed to prove generic path behavior and deterministic test fixtures.

Initial concepts:

```text
VirtualPath
NormalizedPath
MountId
MountTable
FileReadRequest
FileReadResult
FileError
FileSystemSnapshot
LooseFileSource
```

Names may change during implementation, but the responsibilities must remain equivalent.

## Ownership

`YuFile` owns:

- virtual path syntax and normalization;
- mount registration and lookup order;
- loose-file existence/read behavior for approved fixtures;
- file result and error values;
- deterministic file/path performance signals.

`YuFile` does not own:

- resource asset identity;
- package file format parsing;
- dependency graph;
- cache/invalidation policy;
- GPU/audio upload scheduling;
- script VM loading;
- editor import/export tools;
- original-game resource meaning.

## Path Rules

The first path model is engine-owned and platform-neutral.

Rules:

- Accepted virtual paths use `/` separators.
- Repeated separators are normalized.
- `.` path segments are removed.
- `..` traversal outside the mounted root is rejected.
- Empty paths and absolute OS paths are rejected.
- Case policy is explicit and stable for the fixture.
- Normalization happens on setup/load paths, not in hot per-frame paths.
- Normalized path values may be compared without reparsing the original string.

This ADR does not require final Unicode, case-folding, or platform-drive semantics. Those require later platform/file ADRs before shipping.

## Mount Rules

The first mount table is deterministic.

Rules:

- Mount IDs are stable setup-path identifiers.
- Mount priority/order is explicit.
- Duplicate mount IDs return explicit error.
- Missing mount returns explicit error.
- Lookup returns a source plus normalized relative path.
- Runtime behavior does not depend on logs or reports.

P1-GATE-005 may support only loose test-fixture mounts. Pack/rpack mounts are deferred.

## Read Rules

First-slice read behavior is synchronous and fixture-sized.

Allowed:

- read a small deterministic test fixture from a loose mounted source;
- return bytes or string_view-equivalent test data through an explicit result;
- return explicit not-found, invalid-path, path-escape, duplicate-mount, and read-failure errors.

Blocked:

- async read;
- package parsing;
- memory-mapped file API;
- streaming API;
- compression/decompression;
- resource handle creation;
- original resource file reads in fast gate;
- JSON/schema parsing as file module behavior.

## Evidence Boundary

TouhouNewWorld `resource/pack*.pak`, `resource/rpack*.dat`, loose JSON, shader, and DB files are evidence for future File/VFS and Resource gates. They are not inputs to P1-GATE-005 fast tests.

Future evidence use must follow this sequence:

1. generic File/VFS boundary accepted and tested;
2. package format ADR/gate states the generic pack responsibility;
3. evidence owner supplies original-game facts as validation fixtures;
4. Resource module owns asset identity and dependency lifetime separately.

Original-game facts must not change path normalization, mount ownership, or read API shape by convenience.

## Performance Requirements

Required deterministic signals:

- path normalization count;
- rejected invalid path count;
- mount count;
- lookup count;
- read byte count for the fixed fixture;
- maximum fixture path length;
- allocation/accounting status or explicit deferral to the owning memory gate;
- sync read result status.

Performance rules:

- Path normalization is setup/load path work.
- Hot paths must use normalized handles/paths, not repeated string normalization.
- No unbounded cache or global mutable resource map is introduced.
- File read fixtures are small and deterministic.
- Diagnostics/logging disabled behavior must not change results.

## Memory And Thread Boundary

Memory:

- path and read buffers have explicit owner and lifetime;
- fixture byte count is bounded;
- if `YuMemory` exists, allocation signals should use its vocabulary;
- if not integrated, the limitation must be explicit and cannot be counted as zero.

Thread:

- P1-GATE-005 uses synchronous calls only.
- Async IO requires accepted Thread/Task queue and shutdown semantics.
- No background file thread, job, callback, or queue is introduced by this ADR.

## Test Requirements

First-slice tests must cover:

- normal path normalization;
- repeated separator and `.` segment normalization;
- `..` traversal rejection;
- empty path and absolute path rejection;
- duplicate mount rejection;
- missing mount / missing file explicit error;
- deterministic mount priority or order;
- loose fixture read returns exact bytes;
- read byte count is recorded;
- disabled diagnostics/logging does not change file results;
- no original-game resource file is required by fast tests.

## P1-GATE-005 Compatibility

P1-GATE-005 may implement:

- `YuFile` target;
- path value/result/error types;
- path normalization;
- deterministic mount table;
- loose fixture source;
- synchronous small fixture read;
- tests under `tests/file`;
- test fixture files under `tests/fixtures/file`.

P1-GATE-005 must not implement:

- package parser for `.pak`, `.dat`, or any original resource format;
- resource handles;
- dependency graph;
- cache/invalidation layer;
- async IO;
- compression/decompression;
- memory-mapped file layer;
- editor import/export;
- report/oracle/capture behavior;
- original-game adapter behavior.

## Non-Goals

This ADR does not decide:

- final package format;
- archive index schema;
- resource handle lifetime;
- asset dependency graph;
- file watching;
- patch/DLC mounting;
- async IO;
- streaming;
- compression;
- platform-specific Unicode/case policy for shipping.

Those require later ADRs and gates.

## Gate Impact

If accepted, ADR-0008 becomes the architecture input for P1-GATE-005 File Primitive And Path Normalization.

If rejected, File/VFS implementation work remains blocked and future resource/package work cannot proceed.
