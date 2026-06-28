# YuEngine Documentation Entry Point

Status: canonical documentation handoff
Owner: Architect
Last major planning sync: `origin/main@705f8ba94fee8ccbb9330d2c37f14bb47114e0d1`

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
- 6 GB plus packed resources;
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
- VQ evidence consistency audit `aa4ea04f` remains held until the human lead
  resumes coordination;
- the coordinator goal is paused/report-only by human instruction;
- no next implementation lane is open.

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

## 8. Resume Rule

The coordinator goal is paused because the human lead asked for report-first
planning. Resume execution only after the human lead explicitly says to continue
coordination.
