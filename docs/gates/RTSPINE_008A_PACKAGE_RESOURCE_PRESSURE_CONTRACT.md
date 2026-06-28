# RTSPINE-008A: Package Resource Pressure Contract

Status: Spec gate accepted
Requested decision: `APPROVED_FOR_NEXT_SPLIT`
Current decision: `APPROVED_FOR_NEXT_SPLIT`
Owner: 八云紫
Implementer: 大妖精
Depends on: RTSPINE-004 implementation commit, RTSPINE-008 read-only decomposition
Source baseline: `ebe9ea35f531aa40133262b701e5e751f8ed9ccf`
Related task: `1323072b-bbf3-4af4-8d7f-7f9936041adb`

## Layer

RTSPINE-008A defines the Package/Resource pressure contract vocabulary for
future RTSPINE-008 write lanes. It is docs/spec only. It does not implement
Package, Resource, File/VFS, RuntimeAsset, Asset, RenderScene, RenderCore, RHI,
Audio, World, editor, Web, UI, external authoring, original-game adapters, or
runtime compatibility code.

```text
shipped-content pressure examples
-> explicit budget assumptions
-> package index and byte-range contract
-> hash coverage and dependency table contract
-> File/VFS ranged IO contract
-> Resource payload window/reference contract
-> RuntimeAsset packaged validation and transaction proof
```

## Current Reality

The current mainline already has deterministic first-slice Package, File/VFS,
Resource, and RuntimeAsset evidence. Those slices use small fixture limits:

- Package entries, dependencies, and load plans are bounded value tables;
- Package entry byte offsets and byte sizes still expose legacy 32-bit mirror
  fields, but RTSPINE-008B uses 64-bit archive byte ranges as the pressure
  authority;
- File/VFS fixture reads and writes are bounded whole-file operations;
- Resource cache and decoded payload paths copy bounded byte arrays into fixed
  test-sized storage;
- RuntimeAsset packaged smoke validates package/load-plan shape, but does not
  yet consume archive byte ranges or package hash coverage as production
  pressure evidence.

Those facts are good deterministic gates. They are not a shipped-content
pressure contract by themselves.

## Pressure Vocabulary

All follow-up RTSPINE-008 work must use these terms consistently:

| Term | Meaning |
| --- | --- |
| `fixture cap` | A small deterministic test or storage limit used to keep unit tests bounded. It is not a production promise. |
| `budget assumption` | An explicit byte, count, dependency, window, or table budget chosen by a gate and enforced by status returns. |
| `pressure example` | Shipped-content-scale data used to choose budgets and risk checks. It is not a hard pass/fail size. |
| `runtime cap` | A hard runtime boundary that rejects input with an explicit status before mutation. |
| `window` | A bounded byte range that may be staged, hashed, decoded, or uploaded without loading a whole archive. |
| `evidence threshold` | The focused test, command, or document row that proves one pressure contract. |

Forbidden wording for new acceptance rows:

- a hard product-byte target with no explicit budget assumption;
- proof by original game package compatibility;
- proof by manual inspection, screenshots, reports, logs, or generated images;
- proof that bypasses Package, File/VFS, Resource, or RuntimeAsset contracts.

## Byte-Range Decision

Future Package/Resource pressure gates must represent archive byte ranges with
unsigned 64-bit offset and size values. A later implementation may keep small
fixture caps, but it must name them as fixture caps and reject overflow before
state mutation.

Minimum byte-range rules:

- offset and size are validated as a pair before Package registry mutation;
- zero-size payload records are rejected unless a later gate explicitly names a
  metadata-only record kind;
- offset plus size overflow is rejected with an explicit status;
- window size is validated against the current budget assumption before File,
  Resource, or RuntimeAsset output is published;
- range validation is deterministic and does not require a real shipped archive.

## Hash Coverage

Future hash coverage must name which bytes and tables are covered. The minimum
set is:

- entry payload hash;
- entry metadata hash;
- dependency table hash;
- package table hash;
- optional archive hash when a later gate introduces a package container.

Hash validation must run before Resource, Asset, RenderScene, RenderCore, RHI,
Audio, World, or RuntimeAsset output mutation. A missing or mismatched hash is
a data-contract failure, not an environment blocker.

## Mutation Contract

All follow-up RTSPINE-008 implementations must validate before mutation. If a
future gate cannot prove every post-preflight failure is impossible, it must
add a rollback or commit-journal proof before claiming production pressure.

No-mutation evidence must use snapshots, caller-owned output sentinels, or
equivalent deterministic counters. It must not rely on logs, sleeps, manual
visual inspection, or broad CTest claims that were not actually run.

## Gate Split

The follow-up split from this contract is:

| Gate | Scope | Release condition |
| --- | --- | --- |
| RTSPINE-008B | Package archive byte-range and index metadata | Accepted by `docs/gates/RTSPINE_008B_PACKAGE_BYTE_RANGE_LEGACY_MIRROR_DECISION.md`; `byte_offset`/`byte_size` are legacy mirrors only |
| RTSPINE-008C | Package artifact hash and dependency integrity | PASS at `origin/main@d18f1679ebd389ecec506055764602591f5b9ab6`; Package-only payload, metadata, dependency table, and package table hash validation |
| RTSPINE-008D | File/VFS ranged IO | PASS at `origin/main@c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e`; File/VFS ranged request/status, no-mutation, and async small-output contract |
| RTSPINE-008E | Resource payload window/reference budget | After this spec, independent from RuntimeAsset files |
| RTSPINE-008F | Package dependency closure and budgeted load plan | After Package metadata/hash evidence |
| RTSPINE-008G | RuntimeAsset packaged validation bridge | After RTSPINE-004 implementation and QA are stable, and after lower Package/File/Resource gates have evidence |
| RTSPINE-008H | RuntimeAsset transaction rollback/proof | After RTSPINE-008G |

RTSPINE-008C focused QA task `ba135e38-b73e-4294-b449-97a04b33b982` reports
`YuPackageTests` build PASS, `^Package_` discovery/execution `35/35` PASS, exact
new integrity rows `2/2` PASS, `git diff --check` PASS, added-line hygiene PASS,
Package-only boundary scans PASS, and no broad/full CTest. This 008C evidence
did not itself release File/VFS; the separate 008D gate below does. It still
does not release RTSPINE-008E/F/G/H, Resource payload windows, or RuntimeAsset
packaged validation.

RTSPINE-008D focused QA task `aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports
`YuFileTests` build PASS, `^File_` discovery/execution `23/23` PASS, ranged
subset `4/4` PASS, `git diff --check` PASS, added-line hygiene PASS, File-only
boundary scans PASS, and no broad/full CTest. This releases only the File/VFS
ranged IO contract and does not release RTSPINE-008E/F/G/H, Resource payload
windows, or RuntimeAsset packaged validation.

## Forbidden Scope

RTSPINE-008A does not authorize:

- Package or Resource implementation;
- RuntimeAssetData, CMake, or RuntimeAssetDataClosedLoopTests edits;
- RTSPINE-005, RTSPINE-006, or RTSPINE-007 implementation;
- WorldObject, editor object, raw pointer, display name, or file path binding;
- RVF, Web, UI, editor, external authoring, or original-game adapters;
- unbudgeted shipped-content byte targets without explicit budget assumptions.

## Acceptance

This gate is accepted when:

1. this document exists as the RTSPINE-008A pressure-contract anchor;
2. `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` links RTSPINE-008 to this contract;
3. `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md` records the same
   vocabulary, byte-range, hash, and no-mutation decisions;
4. a stale hard-coded size target scan finds no current hard shipped-content byte
   target in the active docs;
5. the diff is docs-only and `git diff --check` passes.
