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
-> RuntimeAsset packaged validation bridge
-> RuntimeAsset transaction proof
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
| RTSPINE-008E | Resource payload window/reference budget | PASS at `origin/main@8bb8eff9c98d2a0aa5050c5da6ad94049fa894be`; Resource cache/decoded payload window and reference-budget contract |
| RTSPINE-008F | Package dependency closure and budgeted load plan | PASS at `origin/main@8509f7e1b6ba15e79c574357a465ddfff4d80e10`; Package dependency closure, de-dup/order, record budget, and archive byte budget no-mutation evidence |
| RTSPINE-008G | RuntimeAsset packaged validation bridge | PASS at `origin/main@175b6542cf8460b279d1de8a5499e2cbd508c80a`; RuntimeAsset packaged archive byte-range/hash preflight, payload hash validation, duplicate load-plan rejection, and ProductRun validation failure reporting without graph mutation |
| RTSPINE-008H | RuntimeAsset transaction rollback/proof | After RTSPINE-008G docs/VQ closure |

RTSPINE-008C focused QA task `ba135e38-b73e-4294-b449-97a04b33b982` reports
`YuPackageTests` build PASS, `^Package_` discovery/execution `35/35` PASS, exact
new integrity rows `2/2` PASS, `git diff --check` PASS, added-line hygiene PASS,
Package-only boundary scans PASS, and no broad/full CTest. This 008C evidence
did not itself release File/VFS or Resource payload windows; the separate 008D
and 008E gates below do, and the separate 008F gate below covers Package
dependency closure. It did not itself release the RuntimeAsset packaged
validation bridge; the separate 008G gate below does. It does not release
RTSPINE-008H.

RTSPINE-008D focused QA task `aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports
`YuFileTests` build PASS, `^File_` discovery/execution `23/23` PASS, ranged
subset `4/4` PASS, `git diff --check` PASS, added-line hygiene PASS, File-only
boundary scans PASS, and no broad/full CTest. This releases only the File/VFS
ranged IO contract. It did not itself release Resource payload windows; the
separate 008E gate below does. It does not release RTSPINE-008H or RuntimeAsset
packaged validation bridge by itself; the separate 008G gate below does.

RTSPINE-008E focused QA task `b4fa51c3-aefc-4714-b5d8-062f8a933ac9` reports
`YuResourceTests` build PASS, Resource window/reference discovery exactly `7`
rows, execution `7/7` PASS, commit-level `git diff --check` PASS, added-line
hygiene PASS, non-goal boundary scans PASS, and no broad/full CTest. This
releases only the Resource payload window/reference budget contract and does not
release RTSPINE-008H or RuntimeAsset packaged validation bridge by itself.

RTSPINE-008F focused QA task `4f199c8e-99a4-43b4-a776-8960285ffdaf` reports
allowed Package/CMake scope, `YuPackageTests` build PASS, exact 008F rows `4/4`
PASS, `^Package_` focused suite `39/39` PASS, `git diff --check` PASS,
added-line hygiene/scope scan PASS, no broad/full CTest, and no QA edits,
staging, or commits. This releases only the Package dependency closure and
budgeted load-plan contract and does not release RTSPINE-008H or RuntimeAsset
packaged validation bridge by itself.

RTSPINE-008G focused QA task `35fdc7a2-c09d-416a-95aa-b4aabdb05d0f` reports
`origin/main@175b6542cf8460b279d1de8a5499e2cbd508c80a` RuntimeAsset packaged
validation bridge PASS: focused `YuRuntimeAssetDataClosedLoopTests` build PASS,
exact RTSPINE-008G rows `5/5` PASS, adjacent packaged/product rows `8/8` PASS,
committed scope limited to `CMakeLists.txt`, `RuntimeAssetData.h/.cpp`, and
`RuntimeAssetDataClosedLoopTests.cpp`, `git diff --check` PASS, no broad/full
CTest, and clean read-only QA. This releases only the RuntimeAsset packaged
validation bridge and does not release RTSPINE-008H transaction rollback/proof,
broader Resource/File/VFS follow-through, WorldObject/editor/importer/RHI/
RenderScene expansion, or unrelated animation mapping.

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
