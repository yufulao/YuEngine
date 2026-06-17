# P2-GATE-015: Package Resource Staging Queue

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-003, P2-GATE-010, P2-GATE-014, P1-GATE-006
Related decisions: ADR-0013
Source baseline: `49a14ae`
Proposal commit: `9f2e5cd`

## Layer

L4-L5 lower-engine package/resource staging boundary over existing File async
and Package load-plan values.

This gate proposes the first bounded staging queue that can turn existing
`PackageLoadPlanRecord` values, caller-owned Resource handles/types, and
caller-owned File async read storage into deterministic resource staging
requests and completions. It is not a package-file parser, Resource loader,
image decoder, RHI upload executor, RenderCore pass, material system, scene
streaming, UI, World, report, screenshot, manual proof, or Game Adapter gate.

```text
PackageLoadPlanRecord values
-> Resource handle/type validation values
-> caller-owned File async read queue request/completion values
-> bounded package-resource staging queue records
-> later decode, Resource load completion, RHI upload, RenderCore scheduling
```

## Current Reality

P2-GATE-003 closed the first `YuPackage` manifest and load-plan boundary without
File reads or Resource registry mutation. P1-GATE-006 provided `YuResource`
identity, handle, type, acquire/release, and dependency vocabulary. P2-GATE-010
landed bounded worker and async File read completion storage. P2-GATE-014
landed RHI texture sampling proof, but the engine still has no package/resource
streaming bridge, no Resource load completion queue, no upload staging records,
no image decode, no RHI upload scheduling, no RenderCore pass scheduling, no
material system, and no scene-driven resource streaming.

Current discovery on the proposal baseline:

- `ctest --preset windows-fast-gate -N`: `726`;
- `ctest --preset windows-fast-gate -N -L Resource`: `18`;
- `ctest --preset windows-fast-gate -N -L Package`: `24`;
- `ctest --preset windows-fast-gate -N -L File`: `16`;
- `ctest --preset windows-fast-gate -N -L AsyncIO`: `10`;
- `ctest --preset windows-fast-gate -N -L Fast`: `726`;
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`: `50`;
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`: `131`;
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`: `0`;
- `ctest --preset windows-fast-gate -N -L Streaming`: `0`;
- `ctest --preset windows-fast-gate -N -L Upload`: `0`;
- `ctest --preset windows-hardware-smoke -N`: `7`;
- `ctest --preset windows-hardware-smoke -N -L Streaming`: `0`;
- `ctest --preset windows-hardware-smoke -N -L Upload`: `0`.

## Approval Evidence

Approved after:

- ENG-121A boundary and quality review PASS with no `NEEDS_ARCHITECTURE`
  blocker;
- ENG-121B implementability review PASS with no `NEEDS_IMPLEMENTABILITY`
  blocker;
- ENG-121C test and preset review PASS with no `NEEDS_TEST_POLICY` blocker.

Review evidence:

- proposal commit `9f2e5cd39d3e0a15f4f8a830540fed78f594a2b9` changes only
  `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` and this gate doc;
- `git diff --check 9f2e5cd^ 9f2e5cd` passed;
- review worktrees stayed clean and reviewers made no source, doc, commit, or
  push changes;
- baseline discovery is default `726`, `Resource` `18`, `Package` `24`, `File`
  `16`, `AsyncIO` `10`, `Fast` `726`, `PerformanceSmoke` `50`,
  `EvidenceOracle` `131`, `HardwareSmoke` `0`, `Streaming` `0`, and `Upload`
  `0`;
- `windows-hardware-smoke` discovers `7`; package/resource/file/async/staging
  and upload labels remain `0` for this proposal baseline.

Approval conditions:

- implementation must remain a narrow staging bridge over existing `YuPackage`
  load-plan values, `YuResource` handle/type validation values, and `YuFile`
  async request/completion values;
- `PackageSourceKey` to `FileReadRequest` mapping must stay caller-provided or
  staging-bridge descriptor-owned, not become a package parser;
- implementation must not mutate Package or Resource core ownership, add File
  dependencies to Resource core, or add Resource mutation to Package core;
- staging completions must remain value/status records and must not mutate
  Resource load completion state;
- default `windows-fast-gate` must remain deterministic and no-real-device;
- no hardware-smoke admission is expected for this gate;
- proof must use deterministic value/counter/status assertions and fixture File
  reads only through deterministic `Tests/Fixtures/File` style data;
- proof must reject screenshots, reports, generated image artifacts, logs,
  sleeps, manual visual inspection, original-game packages/output, silent skip,
  RHI upload, RenderCore draw, material bind, scene streaming, or Game Adapter
  behavior.

## Owns

This gate owns the proposal for:

- a narrow staging bridge module that depends on existing public `YuPackage`,
  `YuResource`, and `YuFile` values without moving those dependencies into
  Package or Resource core;
- value-only staging descriptors for package entry, expected resource type,
  logical key, byte range, destination resource handle, request index, and
  caller-owned output storage;
- bounded queue records for pending staging work and completion records;
- validation for null inputs, stale Resource handles, type mismatch, byte range
  overflow, queue capacity overflow, completion capacity overflow, duplicate
  request ids, and File async completion failures;
- deterministic tests under default `windows-fast-gate` with `Streaming`,
  `Upload`, `Resource`, `Package`, `File`, and `AsyncIO` labels;
- snapshot counters/statuses proving bounded queue behavior and no mutation on
  failed validation.

## Does Not Own

This gate does not own:

- package file parsing, original package format readers, compression, archive
  indexing, pack/rpack compatibility, or original-game output proof;
- image decode, texture import, audio decode, mesh decode, shader compile,
  material graph, or asset cache ownership;
- Resource load completion mutation, acquire/release policy changes, Resource
  retirement policy changes, or ResourceRegistry storage ownership;
- RHI texture/buffer creation, GPU upload execution, command lists, fences,
  RenderCore scheduling, material binding, scene traversal, UI, World, Script,
  gameplay, reports, screenshots, manual visual proof, or Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Package owns metadata and deterministic entry/load-plan lookup;
- File owns path normalization, mounted reads, and async read completions;
- Resource owns identity, handle lifetime, dependency, and acquire/release
  vocabulary;
- the proposed staging bridge owns only the bounded cross-module request and
  completion records needed to connect those existing values;
- RHI and RenderCore later own GPU upload execution and render submission;
- World and Game Adapter later own scene and gameplay meaning.

YuEngine must not copy UE or Unity source, API names, private layout, or module
names. This gate uses those engines only to keep package metadata, file IO,
resource identity, decode, upload execution, render scheduling, and gameplay
meaning separate.

## Dependencies

Allowed dependencies:

- existing `YuPackage` public load-plan value contracts;
- existing `YuResource` public handle/type/status and const validation
  contracts;
- existing `YuFile` public async read request/completion value contracts;
- a new narrow staging bridge module or equivalent isolated bridge surface;
- `Tests/Streaming` or equivalent focused tests plus root CMake labels;
- this gate and queue documentation.

Forbidden dependencies:

- RHI, D3D11, DXGI, Platform, RenderCore, material, scene, UI, World, Script,
  Game Adapter, report, screenshot, visual proof, shader compiler, shader
  source tooling, image decoder, audio decoder, mesh decoder, importer, original
  package parser, or original-game evidence;
- mutating `YuPackage` or `YuResource` core to include File ownership;
- adding File dependencies to Resource core or Resource mutation to Package core;
- generated reports, screenshots, manual visual inspection, sleep timing, logs,
  or original-game output as evidence.

## Public Contract Boundary

Public staging contracts may expose value-only descriptors such as:

- package id, package entry id, resource logical key, resource type id, byte
  offset, and byte size copied from existing `PackageLoadPlanRecord`;
- `ResourceHandle` and expected `ResourceTypeId` values for validation;
- File mount/path request values and caller-owned byte output storage;
- fixed-capacity queue and completion snapshots.

Public staging contracts must not expose:

- native file handles, Windows handles, RHI handles, D3D11/DXGI/COM types,
  RenderCore pass ids, material ids, scene ids, UI ids, report handles,
  screenshot artifacts, visual-proof types, or Game Adapter types;
- package archive internals, decoded image/audio/mesh payload formats, shader
  source/compiler handles, or original-game package layout details.

## First Slice Shape

If approved, the first implementation slice should:

1. Add only value contracts and bounded queue storage for package-resource
   staging requests and completions.
2. Use existing package load-plan record values as input; do not parse package
   files or original package formats.
3. Use existing Resource handle/type validation without mutating Resource
   acquire/release or load state.
4. Use existing File async request/completion values through caller-owned queue
   references and caller-owned output storage.
5. Add deterministic fast tests for success, invalid handles, type mismatch,
   byte range overflow, missing File completion, queue overflow, completion
   overflow, duplicate request ids, and snapshot counters.
6. Keep all tests in default `windows-fast-gate`; no hardware-smoke admission is
   expected for this gate.

## Test And Evidence Policy

Default `windows-fast-gate` must remain deterministic and must not require real
hardware, real package files, original-game packages, screenshots, reports, or
manual inspection.

Required implementation evidence:

- `cmake --preset windows-fast-gate`;
- `cmake --build --preset windows-fast-gate`;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- discovery counts for `Resource`, `Package`, `File`, `AsyncIO`, `Streaming`,
  `Upload`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and `HardwareSmoke`;
- proof that `windows-hardware-smoke` remains unaffected or has no
  package/resource staging tests;
- public-header scan for native/RHI/RenderCore/World/Game leakage;
- production dependency scan for forbidden upper modules and original-game
  package parser/decode/upload execution dependencies;
- changed-path and `CMakePresets.json` no-drift checks;
- proof-shape scan rejecting screenshots, reports, logs, sleeps, manual visual
  proof, file proof based on original-game outputs, and silent skip.

Accepted proof:

- deterministic value/counter/status assertions in fast tests;
- fixture File reads only through existing deterministic `Tests/Fixtures/File`
  style data;
- explicit capacity and validation counters proving failed paths do not mutate
  queue or registry state.

Rejected proof:

- screenshots, reports, generated image artifacts, manual visual inspection,
  logs, sleeps, visual demos, original-game packages, or original-game output;
- silent skip of staging proof;
- RHI upload, RenderCore draw, material bind, scene streaming, or Game Adapter
  behavior as evidence for this gate.

## Non Goals

- No original package parser.
- No Resource load completion state machine.
- No image, audio, mesh, shader, or material decoder.
- No RHI upload execution or GPU resource creation.
- No RenderCore pass scheduling.
- No UI, World, Script, gameplay, report, screenshot, manual visual proof, or
  Game Adapter behavior.

## Review Request

Request `APPROVED_FOR_FIRST_SLICE` only after:

1. boundary review confirms the scope stays in a narrow staging bridge plus
   public `YuPackage`, `YuResource`, and `YuFile` values without mutating
   Package or Resource core ownership;
2. implementability review confirms existing Package load-plan, Resource
   validation, and File async queue contracts can support the first slice
   locally;
3. test-policy review confirms default fast-gate determinism, label coverage,
   and proof requirements without hardware, reports, screenshots, manual proof,
   original-game package outputs, or silent skip.
