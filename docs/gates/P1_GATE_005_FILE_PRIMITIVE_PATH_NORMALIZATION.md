# P1-GATE-005: File Primitive And Path Normalization

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 射命丸文, 博丽灵梦, 雾雨魔理沙 when implementation exists
Depends on: ADR-0008
Related decisions: ADR-0001, ADR-0002, ADR-0005, ADR-0006
Source baseline: Phase 1 through `2a99a2a`

## Layer

L1-L3:

- L1 file/path primitive behavior.
- L3 VFS lookup boundary for loose fixture sources only.

## Owns

This gate owns the first implementation slice for:

- `YuFile` CMake target;
- virtual path and normalized path values;
- file result/error values;
- deterministic path normalization;
- bounded mount table;
- loose fixture source;
- synchronous small fixture reads;
- file/path tests.

## Does Not Own

This gate does not own:

- package parser for `.pak`, `.dat`, or any original resource format;
- resource handles;
- asset dependency graph;
- cache or invalidation policy;
- async IO;
- thread/task queue;
- streaming;
- compression/decompression;
- memory-mapped files;
- shader/script/audio/texture/resource semantics;
- import/export tools;
- report/capture/oracle output;
- game adapter behavior.

## UE/Unity Analogue

UE5 references:

- `Runtime\PakFile`, `Runtime\AssetRegistry`, and project/package boundaries as responsibility references.

Unity references:

- Asset Database, Package Manager, AssetBundles, and import pipeline as workflow references.

YuEngine decision:

- Start with generic path and loose fixture behavior.
- Keep VFS/package lookup separate from Resource handles and upload scheduling.
- Keep original-game resource facts out of L1-L3 API shape.

## Lifecycle

First slice lifecycle:

1. Setup creates a bounded mount table.
2. Setup registers a loose test-fixture mount.
3. File request normalizes a virtual path.
4. Lookup resolves mount plus normalized relative path.
5. Sync read returns fixture bytes or explicit error.
6. Snapshot records normalization, lookup, byte count, and error signals.

Failure behavior:

- invalid path returns explicit invalid-path error;
- path traversal outside mount returns explicit path-escape error;
- duplicate mount returns explicit duplicate-mount error;
- missing mount/file returns explicit not-found error;
- read failure returns explicit read-failure error;
- no failure is detected only through log text.

## Inputs

- fixed virtual paths;
- fixed mount IDs;
- small test fixture files under `tests/fixtures/file`;
- optional memory tracker if P1-GATE-002 implementation exists.

## Outputs

- normalized path values;
- file read result;
- file error/status values;
- file snapshot/counters;
- no JSON report output as runtime API.

`FileReadResult` must either own its byte buffer or expose a view tied to a
source/result lifetime that cannot dangle. Error details cross the boundary as
explicit result/status values, not log or report text.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` only if P1-GATE-002 implementation has landed and integration is limited to allocation/byte fixture signals;
- `YuDiagnostics` only in tests if needed to prove diagnostics-disabled behavior.

Target dependency expectation:

```text
YuFile
  -> optional YuMemory after P1-GATE-002 implementation
```

`YuFile` must not depend on `YuKernel`, `YuThread`, `YuPlatform` beyond standard-library file IO unless a later platform-file ADR approves it, resource/RHI/audio/input/script/world/UI modules, tools, reports, or original-game evidence.

Forbidden dependencies:

- TouhouNewWorld `resource` files in fast tests;
- old `YuEngine_BACKUP` runtime files;
- old `FrameRuntime.*`;
- package/resource semantics;
- diagnostics report schemas;
- async thread/task primitives.

## Performance Constraints

Required deterministic signals:

- path normalization count;
- invalid/rejected path count;
- mount count;
- lookup count;
- read byte count for the fixed fixture;
- maximum fixture path length;
- allocation/accounting status or explicit deferral if `YuMemory` is not integrated;
- sync read result status.

First-slice bounds:

- mount capacity: 4 mounts maximum;
- maximum virtual path length: 128 bytes;
- maximum normalized path length: 128 bytes;
- maximum fixture read size: 4096 bytes;
- first-slice case policy: case-sensitive virtual path comparison, lowercase ASCII fixtures, and no OS case-folding;
- allocation/accounting rule: use `YuMemory` if the P1-GATE-002 implementation has landed; otherwise explicitly defer only the allocation/accounting signal and do not claim zero CRT/STL/general heap coverage.

Pass/fail rule:

- exceeding the path length, mount capacity, or fixture read-size bound is an explicit failure;
- changing mount capacity, path normalization count, lookup count, read byte count, or allocation/accounting status outside the declared fixture is a gate failure unless this gate is amended.

Blocking conditions:

- repeated path normalization in a declared hot path;
- unbounded mount table or cache;
- original-game resource file required by fast gate;
- package parser introduced by this slice;
- async IO or background file thread introduced by this slice;
- diagnostics/report output required for file behavior.

## Tests

Fast gate tests required before the slice can be considered complete:

- `File_PathNormalize_RemovesDotAndRepeatedSeparators`
- `File_PathNormalize_RejectsTraversalOutsideRoot`
- `File_PathNormalize_RejectsEmptyAndAbsolutePath`
- `File_MountTable_RejectsDuplicateMount`
- `File_MountTable_UsesDeterministicPriorityOrder`
- `File_MountTable_ReportsMissingMountOrFile`
- `File_LooseFixtureRead_ReturnsExactBytes`
- `File_ReadSnapshot_RecordsCountsAndBytes`
- `File_DiagnosticsDisabled_DoesNotChangeBehavior`

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
src/yuengine/file/include/yuengine/file/
src/yuengine/file/src/
tests/file/
tests/fixtures/file/
```

It may update root `CMakeLists.txt` only to add `YuFile` and `YuFileTests`.

It may not create placeholder directories or targets for resource, script, RHI, audio, input, world, UI, tools, or game adapter work.

## Non-Goals

- No package parsing.
- No Resource module.
- No original resource reading in fast gate.
- No async IO.
- No cache/invalidation layer.
- No file watcher.
- No compression/decompression.
- No import/export tool.
- No report/oracle/capture behavior.

## Evidence Inputs

No original-game evidence is required for the first slice.

Known TouhouNewWorld files such as `resource/pack01.pak`, `resource/rpack01.dat`, loose JSON, DB, shader, and effect files remain future evidence for package/resource gates only. They must not be read by P1-GATE-005 fast tests.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0008 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements;
- 射命丸文 confirms the evidence boundary does not misuse original resource facts;
- 博丽灵梦 confirms path/read performance signals and hot-path string policy are sufficient;
- 雾雨魔理沙 confirms first-slice implementation review can enforce boundaries and tests.

If those conditions are not met, return `NEEDS_ARCHITECTURE`, `NEEDS_EVIDENCE`, or `NEEDS_PERFORMANCE` with exact missing fields.
