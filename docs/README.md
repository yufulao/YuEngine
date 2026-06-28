# YuEngine Documentation Entry Point

Status: canonical documentation handoff
Owner: Architect
Last major planning sync: `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`

## 1. Read This First

This directory contains many historical plans, review notes, RAV records, editor
workflows, and evidence snapshots. They are not all current instructions.

A new team, a restarted agent, or a context-compressed session must start from
this file and the canonical documents below. Do not infer current direction from
older `RAV`, `EDITOR`, `BRIDGE`, or per-feature documents unless one of the
canonical documents explicitly points to them.

## 2. Current Product Target

YuEngine is a small-team native commercial game engine. The current product
reference bar is the shipped TouhouNewWorld class of engine:

- small native runtime binaries;
- no Unity/UE runtime shape as the target architecture;
- shipped-content-scale packed resources as a pressure example, not a hard byte
  target;
- long-session stability around the 20 hour gameplay class;
- package/resource indexes, runtime asset records, diagnostics, and toolchain
  evidence strong enough for a real shipped game.

The goal is not broad UE/Unity feature parity. The goal is a narrower native
engine that the team can ship, patch, diagnose, and maintain.

## 3. Authoritative Current Documents

Read in this order:

| Order | Document | Purpose |
| --- | --- | --- |
| 1 | `docs/YUENGINE_LONG_PLAN_TEAM_EXECUTION.md` | Final target, non-negotiable principles, long-horizon roadmap, current nearest stage, stop conditions |
| 2 | `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` | L0/L1 execution plan, production target, long-roadmap summary, immediate RTSPINE backlog |
| 3 | `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md` | RuntimeAsset production spine, file/source/cooked contracts, asset target identity before deeper animation |
| 4 | `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md` | Current L1 ownership matrix, state corrections, animation runtime foundation downgraded to `FirstSlice` |
| 5 | `docs/YUENGINE_RUNTIME_VISUAL_EVIDENCE_MATRIX.md` | Runtime visual evidence status and current proof links |

If these documents conflict with older docs, these documents win.

## 4. Current Execution State

At the latest handoff:

- scene-animation implementation is complete at
  `f211f7f95299388987ccef00b4d1e8ee6f7bf0c1`;
- scene-animation QA is PASS;
- docs evidence sync is complete at
  `0a9144b0e30cbede56a5dbf04b232f3e5b763802`;
- long-term planning correction is complete at
  `705f8ba94fee8ccbb9330d2c37f14bb47114e0d1`;
- documentation entry cleanup is complete at
  `b0d96b0dece4009e753dd15307235e2e8b8badac`;
- RTSPINE-003 target identity implementation and focused QA are PASS at
  `5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`;
- RTSPINE-003 focused QA reports the focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact target identity plus
  scene/runtime animation regression discovery `10` rows, execution `10/10`
  PASS, `git diff --check` PASS, added-line hygiene PASS, and boundary scans
  PASS without running broad full CTest;
- RTSPINE-003 docs evidence is synchronized by workspace task
  `d9dc3692-aa12-4f5c-872a-5b7293a92ceb`;
- RTSPINE-003 VQ evidence consistency audit is PASS by workspace task
  `fdd78da4-da12-4956-b6ac-63ff9e377121`;
- RTSPINE-004 animation track target/property binding implementation and focused
  QA are PASS at `ebe9ea35f531aa40133262b701e5e751f8ed9ccf`;
- RTSPINE-004 focused QA task `2e2d5a4e-0bb0-4cf4-bd1b-ab3a87987b7f`
  reports focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact
  discovery `17` rows, execution `17/17` PASS, `git diff --check` PASS,
  added-line hygiene PASS, and boundary/non-goal scans PASS without running
  broad full CTest;
- RTSPINE-004 covers SceneNode `target_id` plus property binding only;
- RTSPINE-005 minimal Step/Linear interpolation implementation and focused QA are
  PASS at `2bfe7e37d36ca711dd706728f21b1e4caecfd3db` /
  `d18f1679ebd389ecec506055764602591f5b9ab6`; focused QA task
  `951a3da8-6b13-4268-960e-407f65c40db7` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact interpolation discovery
  `3`, execution `3/3` PASS, non-Package RuntimeAsset animation whitelist
  `23/23` PASS, diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-006 invalid-target failure model implementation is PASS at
  `96e0c024435f670c39ced019ff825b819a6830a3`; focused QA task
  `6d02c260-936a-456b-917b-5c2802bbb666` reports isolated clean worktree at
  `96e0c024`, focused `YuRuntimeAssetDataClosedLoopTests` build PASS, focused
  RuntimeAsset regex discovery/execution `8/8` PASS, exact new rows `2/2`
  PASS, diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-007 runtime instance mapping implementation is PASS at
  `37a112549190ac2123abcd72b5c688cdfa5b01e5`; focused QA task
  `6b6baf5f-2381-4b9c-89b1-4411fba53d23` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS,
  `RuntimeAssetData_(RuntimeInstanceMapping|AnimationTrackTargetBinding|AnimationFailureModel|RuntimeAnimationTables)`
  discovery/execution `12/12` PASS, exact RuntimeInstanceMapping rows `5/5`
  PASS, commit-level diff/hygiene/boundary PASS, read-only QA with clean final
  repo, and no broad/full CTest;
- WorldObject-facing runtime instance mapping remains unopened;
- the human lead has resumed execution and requires continuous multi-agent
  coordination until the L0/L1 stop condition is actually met;
- RTSPINE-008A docs/spec is PASS at
  `ad1a7fb5b3dfa2e1f118103158b640a7111d767f`; the only current
  Package/Resource write lane is RTSPINE-008B Package byte-range/index;
- RTSPINE-008B keeps `archive_byte_offset` and `archive_byte_size` as the
  authoritative pressure byte range, with `byte_offset` and `byte_size` as
  legacy mirrors only, as recorded in
  `docs/gates/RTSPINE_008B_PACKAGE_BYTE_RANGE_LEGACY_MIRROR_DECISION.md`;
- RTSPINE-008C Package artifact hash/dependency integrity implementation and
  focused QA are PASS at `d18f1679ebd389ecec506055764602591f5b9ab6`; focused
  QA task `ba135e38-b73e-4294-b449-97a04b33b982` reports `YuPackageTests`
  build PASS, `^Package_` discovery/execution `35/35` PASS, exact new integrity
  rows `2/2` PASS, diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-008D File/VFS ranged IO implementation is PASS at
  `c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e`; focused QA task
  `aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports `YuFileTests` build PASS,
  `^File_` discovery/execution `23/23` PASS, ranged subset `4/4` PASS,
  diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-008E Resource payload window/reference budget implementation is PASS
  at `8bb8eff9c98d2a0aa5050c5da6ad94049fa894be`; focused QA task
  `b4fa51c3-aefc-4714-b5d8-062f8a933ac9` reports `YuResourceTests` build
  PASS, Resource window/reference discovery exactly `7` rows, execution `7/7`
  PASS, commit-level diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-008F Package dependency closure and budgeted load plan implementation
  is PASS at `8509f7e1b6ba15e79c574357a465ddfff4d80e10`; focused QA task
  `4f199c8e-99a4-43b4-a776-8960285ffdaf` reports allowed Package/CMake scope,
  `YuPackageTests` build PASS, exact 008F rows `4/4` PASS, `^Package_`
  focused suite `39/39` PASS, diff/hygiene/scope scan PASS, no broad/full
  CTest, and no QA edits/staging/commits;
- RTSPINE-008G RuntimeAsset packaged validation bridge implementation is PASS
  at `175b6542cf8460b279d1de8a5499e2cbd508c80a`; focused QA task
  `35fdc7a2-c09d-416a-95aa-b4aabdb05d0f` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact packaged validation
  rows `5/5` PASS, adjacent packaged/product rows `8/8` PASS, committed scope
  limited to `CMakeLists.txt`, `RuntimeAssetData.h/.cpp`, and
  `RuntimeAssetDataClosedLoopTests.cpp`, `git diff --check` PASS, and no
  broad/full CTest;
- RTSPINE-008H RuntimeAsset transaction rollback/proof implementation is PASS
  at `1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1`; focused QA task
  `1ec65e79-70f2-4fe5-8f08-6fb0ba2371fd` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact transaction rollback
  row `1/1` PASS, focused rollback/commit/adjacent packaged/product set
  `19/19` PASS, committed scope limited to `CMakeLists.txt`,
  `RuntimeAssetData.h/.cpp`, and `RuntimeAssetDataClosedLoopTests.cpp`,
  `git diff --check` PASS, and no broad/full CTest;
- WorldObject-facing runtime instance mapping and broader Resource/File/VFS
  follow-through remain blocked until their own gates are released.

Live workspace state is still authoritative for task ownership and current
status. This file records the handoff baseline, not a replacement for the task
board.

## 5. Current Nearest Stage

The next stage is RuntimeAsset production spine correction. It is not broad
feature expansion.

Required order:

```text
RuntimeAsset container and family identity
-> package/resource index and dependency tables
-> asset-internal scene node / model node / skeleton joint targets
-> animation track/channel binding to target plus property
-> Step/Linear interpolation
-> sampled transform application to runtime instance records
-> WorldObject mapping only after instance contracts exist
-> editor/importer authoring surfaces after runtime contracts pass
```

Animation must not bind directly to WorldObject, editor object, raw pointer,
display name, or file path.

## 6. Historical Documents Policy

Historical docs stay in the repository because they contain evidence and design
context. They do not automatically direct future work.

Treat these categories as historical unless a canonical document explicitly
references them:

- `*_RAV*.md`
- `*_EDITOR_*.md`
- old bridge audits and queue documents;
- old phase documents;
- old preview/resource-browser/UI workflow documents;
- old per-gate plans that predate the current production-spine correction.

Do not delete evidence documents casually. If the team decides to hard-clean the
directory later, do it as a separate docs archival task with a table mapping
each removed or moved document to the canonical document that replaces it.

## 7. Stop Conditions

Stop and return to architecture if any future work:

- bypasses Package/Resource/RuntimeAsset and claims production asset proof;
- binds animation or scene data directly to WorldObject or editor ids;
- treats reports, screenshots, viewers, or logs as runtime behavior;
- opens a new implementation lane before the active evidence gate closes;
- uses old TouhouNewWorld compatibility to define first-class L0/L1 contracts;
- expands editor/UI/gameplay before runtime contracts are stable.

## 8. Continuous Execution Rule

The coordinator must continue driving the accepted plan until the L0/L1 stop
condition is met. Do not stop after one task, and do not wait for the human lead
to say "continue" while accepted work remains.

Required execution discipline:

- keep the architect on coordination, dependency control, design decisions, and
  evidence governance rather than routine frontline implementation;
- split real parallel work only when tasks have independent read or write
  surfaces and do not depend on each other;
- do not create fake parallel QA lanes that only serialize one dependency chain
  or burn review tokens without increasing throughput;
- every shared task must state scope, non-goals, expected evidence, AI ETA, and
  stale-owner timeout behavior;
- set workspace timers for owner checkpoints and reroute stale work instead of
  waiting indefinitely;
- focused evidence is the default; broad full-suite testing is reserved for
  explicit shared-contract, release, or high-risk decisions;
- never open a write lane that can invalidate an active evidence gate.

The current safe pattern is: keep VQ read-only, immediately release the next
non-conflicting lane once its dependency closes, and keep unrelated read-only
architecture, prep, and pressure-audit lanes moving in parallel.
