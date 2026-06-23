# YuEngine RuntimeAsset v0 RAV1 Implementation Evidence

Status: implementation evidence skeleton
Owner: Architecture / Evidence
Task: #61
Baseline: `6acf380 Amend RuntimeAsset RAV1 evidence commands`
Phase A package decision: `APPROVE_RAV1_PHASE_A_DOCS_PACKAGE`
Phase A evidence matrix: `docs/YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md`
Implementation review gate: task #62

## Purpose

This document tracks implementation evidence for the RAV1 RuntimeAsset v0
production slice after the Phase A docs/evidence package passed review.

It is not runtime implementation approval by itself. It is the live matrix that
task #62 reviews once #56 through #60 have concrete commits and test results.

## Implementation Task Matrix

| Task | Scope | Owner input | Current anchor | Required evidence before #62 |
| --- | --- | --- | --- | --- |
| #56 RAV1-I0 | source/cooked parser and validator skeleton | #50 contract, #54 Phase A evidence | pending | focused parser/validator tests, suffix-free type truth, no-mutation failures, changed-path summary |
| #57 RAV1-I1 | loader transaction core and commit semantics | #51 transaction plan, #54 Phase A evidence | pending | preflight/commit tests, `mutated_state` or rollback ledger proof, registry/output no-mutation probes |
| #58 RAV1-I2 | cooked texture/material payload bridge | #52 payload route, #54 Phase A evidence | pending | texture layout/hash/row-pitch tests, material slot resolution tests, RHI texture cleanup/no-output-mutation tests |
| #59 RAV1-I3 | cooked shader/program payload bridge | #52 payload route, #54 Phase A evidence | pending | cooked bytecode descriptor tests, reflection/input-layout tests, module/pipeline cleanup tests |
| #60 RAV1-I4 | bounded scene/animation record loader | #53 scene/animation plan, #54 Phase A evidence | pending | bounded N-entity success, capacity/ref/track/keyframe/target/hash failures without mutation, RenderScene consumption |
| #61 Evidence | implementation evidence matrix and commands | #56-#60 delivery threads | this document | complete command matrix, off-scope scans, suffix scans, changed files, PASS/FAIL/blocker rows |
| #62 Review | implementation review gate | #56-#61 evidence | pending | explicit commit anchor and PASS/AMEND before any next slice opens |

## Command Floor

Every implementation task must report these commands, with the exact commit
anchor used:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

Focused tests must be listed separately. Full fast gate can be deferred only when
a task is docs-only; #56 through #60 are code-bearing implementation tasks and
therefore need build/test evidence before #62 PASS.

## Focused Proof Rows

| Proof area | Minimum focused tests or equivalents | Status |
| --- | --- | --- |
| Source/cooked parser | missing schema, wrong kind, unsupported version, invalid count/size/alignment/hash, misleading suffix | pending |
| Loader transaction | preflight failure no mutation, commit failure `mutated_state`, dependency/decoded-payload intent ordering | pending |
| Texture/material payload | cooked texture layout/hash/row pitch, slot resolution, invalid payload no output mutation, RHI texture cleanup | pending |
| Shader/program payload | cooked stage bytecode, reflection/input-layout, hash/stage mismatch no mutation, module/pipeline cleanup | pending |
| Scene/animation loader | bounded N entities, capacity overflow, invalid transforms/keyframes, target mismatch, path independence, RenderScene consumption | pending |
| Final route | File/Mount/VFS -> Resource/Asset -> RenderScene/RenderCore/RHI from loaded RuntimeAsset records | pending |

## Required Scans

Implementation review must include an off-scope scan over touched runtime/docs
paths:

```powershell
rg -n "editor|Editor|Web|UI|input|Game Adapter|original package|TouhouNewWorld package|GDI|screenshot|manual inspection|direct struct" <touched paths>
```

Allowed matches are only exclusions, blockers, tests rejecting bypasses, or
auxiliary-evidence statements.

Suffix/type-truth scan:

```powershell
rg -n "\.yu(mesh|mat|tex|program|scene|anim)|suffix|fixture name|type truth|internal metadata" <touched paths>
```

Allowed meaning: `.yu*` names are smoke locators only; internal
magic/version/kind/schema/id/hash/dependency metadata is authoritative.

## Review Rules For #62

#62 may return PASS only when:

- #56 through #60 have delivery commits or explicitly approved blockers;
- #61 has updated this matrix with actual anchors and command outputs;
- all required command floors pass or failures are named as AMEND blockers;
- no task relies on suffix/path type truth;
- no task counts CPU PPM, GDI/software viewers, screenshots, reports, manual
  inspection, editor/import flow, original package parsing, or direct C++ struct
  injection as production RuntimeAsset proof;
- transaction semantics match #51: pre-commit failures are no-mutation, and any
  post-mutation failure reports `mutated_state == true` unless an approved
  rollback ledger is implemented and tested.

#62 must return AMEND when evidence is missing, commands are vague, mutation
semantics are unproven, or implementation scope expands beyond RuntimeAsset v0
production cook/load/render proof.
