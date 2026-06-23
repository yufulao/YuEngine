# YuEngine RuntimeAsset v0 RAV1 Implementation Evidence

Status: implementation evidence in progress
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
| #56 RAV1-I0 | source/cooked parser and validator skeleton | #50 contract, #54 Phase A evidence | accepted: `232c911 Amend RuntimeAsset header parsing strictness` | focused parser/validator tests, suffix-free type truth, no-mutation failures, changed-path summary |
| #57 RAV1-I1 | loader transaction core and commit semantics | #51 transaction plan, #54 Phase A evidence | accepted: `81c97be Harden RuntimeAsset graph transaction preflight` | preflight/commit tests, `mutated_state` or rollback ledger proof, registry/output no-mutation probes |
| #58 RAV1-I2 | cooked texture/material payload bridge | #52 payload route, #54 Phase A evidence | accepted: `f32ee36 Bridge cooked texture payloads to material slots` | texture layout/hash/row-pitch tests, material slot resolution tests, RHI texture cleanup/no-output-mutation tests |
| #59 RAV1-I3 | cooked shader/program payload bridge | #52 payload route, #54 Phase A evidence | pending | cooked bytecode descriptor tests, reflection/input-layout tests, module/pipeline cleanup tests |
| #60 RAV1-I4 | bounded scene/animation record loader | #53 scene/animation plan, #54 Phase A evidence | accepted: `749e2e6 Implement bounded RuntimeAsset scene animation loader` | bounded N-entity success, capacity/ref/track/keyframe/target/hash failures without mutation, RenderScene consumption |
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

## Recorded Delivery Evidence

### #56 RAV1-I0 Source/Cooked Parser And Validator Skeleton

Accepted anchor: `232c911 Amend RuntimeAsset header parsing strictness`

Implementation note: `232c911` is rebased on `70a2fc8 Implement RuntimeAsset
graph load transaction`. This row accepts only the #56 parser/validator
skeleton and AMEND fix. It does not accept #57 transaction semantics, which
remain pending independent review.

Changed files:

- `CMakeLists.txt`
- `Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp`
- `Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp`

Owner-reported verification at `232c911`:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate -R RuntimeAssetData --output-on-failure
ctest --preset windows-fast-gate --output-on-failure
```

Reported result: diff/show/configure/build PASS, RuntimeAssetData 33/33 PASS,
full fast gate 1282/1282 PASS.

Architecture local spot-check in the detached review worktree:

```powershell
git diff --check 70a2fc8..232c911
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_(HeaderParserRejectsPartialVersionsAndNoise|SourceCookedParserReportsBoundedMetadata|SourceCookedParserRejectsInvalidTablesHashesAndDependencies|LoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation)" --output-on-failure
```

Local result: diff/show/configure/build PASS, focused #56 tests 4/4 PASS.

Scope accepted:

- RuntimeAsset header detection now parses only the first line as strict
  `magic kind version` tokens.
- `magic` must be exactly `YUASSET` or `YUCOOKED`.
- `kind` must match the expected RuntimeAsset family; path suffixes do not
  participate in type recognition.
- parsed `version != 1` returns `UnsupportedVersion`, including versions `3`
  and `10`; prefix/suffix noise and valid-looking later-line headers reject as
  `InvalidHeader`.
- cooked skeleton checks table counts/sizes, supported alignment, source/payload
  hashes, dependency rows, family metadata, and loader pre-commit no-mutation
  for invalid cooked metadata.

Boundary: #56 is a source/cooked parser and validator skeleton only. It is not
a complete production binary parser, payload bridge, transaction approval,
bounded scene/animation loader approval, editor/import bridge, or final render
closure.

### #57 RAV1-I1 Loader Transaction Core And Commit Semantics

Accepted anchors:

- `70a2fc8 Implement RuntimeAsset graph load transaction`
- `81c97be Harden RuntimeAsset graph transaction preflight`

Implementation note: #57 is reviewed as two RuntimeAssetData deltas:
`191f46b..70a2fc8` for the initial transaction implementation and
`232c911..81c97be` for preflight hardening. This row accepts transaction
diagnostics and commit semantics only. It does not accept payload bridges,
bounded scene/animation loader scope, a rollback ledger, or final runtime
render closure.

Changed files:

- `CMakeLists.txt`
- `Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h`
- `Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp`
- `Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp`

Owner-reported verification at `81c97be`:

```powershell
git diff --check origin/main..HEAD -- CMakeLists.txt Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData" --output-on-failure
ctest --preset windows-fast-gate --output-on-failure
```

Reported result: diff/show/configure/build PASS, RuntimeAssetData 34/34 PASS,
full fast gate 1283/1283 PASS.

Architecture local spot-check in the detached review worktree at `97d0364`
(code content includes `81c97be` plus the #61 evidence doc update):

```powershell
git diff --check 191f46b..70a2fc8
git diff --check 232c911..81c97be
git show --check --format=short 81c97be
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_(LoaderRejectsMissingSchemaBeforeMutation|LoaderCommitFailureReportsMutatedState|LoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation)" --output-on-failure
ctest --preset windows-fast-gate -R RuntimeAssetData --output-on-failure
```

Local result: diff/show/configure/build PASS, focused #57 tests 3/3 PASS,
RuntimeAssetData 34/34 PASS.

Scope accepted:

- graph load now builds a request-local transaction plan before runtime
  mutation, with record, Resource/Asset, cache payload, decoded payload, and
  dependency-edge commit counts;
- preflight reads and validates all RuntimeAsset records, scene dependency
  family checks, scene-output staging, request-local ids/capacities, current
  Resource/Asset capacity, cache/decode/decoded payload capacity, and payload
  byte budgets before commit;
- commit order is deterministic: configure budgets, scene Resource/Asset/file
  payloads, file Resource/Asset/payloads, dependency edges, scene output;
- commit phase sets `transaction_result.mutated_state = true` when commit
  begins; no rollback ledger is claimed;
- pre-commit failures leave caller loaded-file outputs, scene output records,
  ResourceRegistry, AssetManager, cache payloads, decoded payloads, dependency
  edges, and RHI snapshots unchanged;
- commit failure diagnostics report `mutated_state == true` and the failing
  commit phase.

Boundary: #57 proves transaction staging/diagnostics for the current
RuntimeAsset graph loader slice only. It is not rollback support, a complete
binary parser, payload bridge approval, scene/animation loader approval, or
final render closure.

### #58 RAV1-I2 Cooked Texture/Material Payload Bridge

Accepted anchor: `f32ee36 Bridge cooked texture payloads to material slots`

Implementation note: #58 is reviewed as independent RuntimeAssetData delta
`97d0364..f32ee36`. Later stack commits are not accepted by this row; in
particular this does not accept #59 shader/program or #60 scene/animation
scope.

Changed files:

- `CMakeLists.txt`
- `Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h`
- `Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp`
- `Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp`

Owner-reported verification at `f32ee36`:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R RuntimeAssetData --output-on-failure
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

Reported result: diff/show/configure/build PASS, RuntimeAssetData 40/40 PASS,
full fast gate 1289/1289 PASS.

Architecture local spot-check in the detached review worktree at `c746573`
(code content includes later #60 implementation and #61 evidence updates):

```powershell
git diff --check 97d0364..f32ee36
git show --check --format=short f32ee36
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_(CookedTexturePayloadTableValidatesLayoutHashAndRowPitch|CookedMaterialTextureSlotTableResolvesLoadedPayloads|CookedPayloadBridgeRejectsTextureFormatExtentSizeAlignmentHashWithoutMutation|CookedPayloadBridgeRejectsMissingDuplicateTypeMismatchDepsWithoutMutation|CookedMaterialSlotOverflowDoesNotMutateRenderSceneOutputs|CookedRhiPartialCreationFailureDestroysTransientHandles)$" --output-on-failure
ctest --preset windows-fast-gate -R RuntimeAssetData --output-on-failure
```

Local result: diff/show/configure/build PASS, focused #58 tests 6/6 PASS,
RuntimeAssetData 45/45 PASS.

Additional local diff-limited scans:

```powershell
git diff --unified=0 97d0364..f32ee36 -- CMakeLists.txt Src\YuEngine\RuntimeAsset\Include\YuEngine\RuntimeAsset\RuntimeAssetData.h Src\YuEngine\RuntimeAsset\Src\RuntimeAssetData.cpp Tests\RenderScene\RuntimeAssetDataClosedLoopTests.cpp | rg -n "editor|Editor|Web|UI|input|Game Adapter|original package|TouhouNewWorld package|GDI|screenshot|manual inspection|direct struct"
git diff --unified=0 97d0364..f32ee36 -- CMakeLists.txt Src\YuEngine\RuntimeAsset\Include\YuEngine\RuntimeAsset\RuntimeAssetData.h Src\YuEngine\RuntimeAsset\Src\RuntimeAssetData.cpp Tests\RenderScene\RuntimeAssetDataClosedLoopTests.cpp | rg -n "\.yu(mesh|mat|tex|program|scene|anim)|suffix|fixture name|type truth|internal metadata"
```

Local result: both diff-limited scans returned no #58 added matches.

Scope accepted:

- cooked texture payload descriptors cover loaded texture identity, RHI texture
  descriptor, color space, row pitch, slice pitch, payload offset/count,
  alignment, payload hash, decoded payload id, staging request id, and upload id;
- cooked texture layout rejects unsupported format, invalid extent, invalid
  size, invalid alignment, nonzero payload offset, missing hash, missing
  staging/upload ids, decoded-payload miss, byte-count mismatch, and payload
  hash mismatch before RHI mutation;
- cooked material slot descriptors resolve typed texture payload refs and reject
  missing refs, duplicate material/texture/sampler slots, duplicate payload refs,
  binding-slot mismatch, unsupported color space, format mismatch, and
  color-space mismatch before RHI mutation;
- successful commit uploads decoded payloads through the Resource/Streaming RHI
  texture bridge, creates samplers, builds a RenderScene runtime material, marks
  texture assets ready, and publishes caller material output;
- partial RHI creation failures report `mutated_state == true`, do not publish
  material output, and clean transient texture/sampler handles with explicit
  cleanup counters.

Boundary: #58 proves the current cooked texture/material bridge route only. It
is not shader/program payload approval, scene/animation loader approval, a
complete binary parser, editor/import bridge, package parser, or final render
closure.

### #60 RAV1-I4 Bounded Scene/Animation Record Loader

Accepted anchor: `749e2e6 Implement bounded RuntimeAsset scene animation loader`

Implementation note: #60 is reviewed as independent RuntimeAssetData delta
`f32ee36..749e2e6`. Later evidence commits are not part of the implementation
scope, and #59 shader/program remains pending independent delivery and review.

Changed files:

- `CMakeLists.txt`
- `Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceConstants.h`
- `Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp`
- `Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp`

Owner-reported verification at `749e2e6`:

```powershell
git diff --check
git show --check --format=short 749e2e6
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_SceneAnimationLoader" --output-on-failure
ctest --preset windows-fast-gate -R "^RuntimeAssetData_" --output-on-failure
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

Reported result: diff/show/configure/build PASS, focused
RuntimeAssetData_SceneAnimationLoader 5/5 PASS, RuntimeAssetData 45/45 PASS,
full fast gate 1294/1294 PASS.

Architecture local spot-check in the detached review worktree at `6cc293c`
(code content includes later #61 evidence updates):

```powershell
git diff --check f32ee36..749e2e6
git show --check --format=short 749e2e6
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_(SceneLoaderRejectsInvalidEntityWithoutOutputMutation|SceneLoaderRejectsInvalidKeyframesWithoutOutputMutation|SceneAnimationLoaderLoadsBoundedNEntityScene|SceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation|SceneAnimationLoaderRejectsMissingRefsWithoutMutation|SceneAnimationLoaderRejectsInvalidRecordsWithoutMutation|SceneAnimationLoaderPathIndependentSceneAnimationDetection)$" --output-on-failure
ctest --preset windows-fast-gate -R RuntimeAssetData --output-on-failure
```

Local result: diff/show/configure/build PASS, focused #60 tests 7/7 PASS,
RuntimeAssetData 45/45 PASS.

Additional local diff-limited scans:

```powershell
git diff --unified=0 f32ee36..749e2e6 -- CMakeLists.txt Src\YuEngine\Resource\Include\YuEngine\Resource\ResourceConstants.h Src\YuEngine\RuntimeAsset\Src\RuntimeAssetData.cpp Tests\RenderScene\RuntimeAssetDataClosedLoopTests.cpp | rg -n "editor|Editor|Web|UI|input|Game Adapter|original package|TouhouNewWorld package|GDI|screenshot|manual inspection|direct struct"
git diff --unified=0 f32ee36..749e2e6 -- CMakeLists.txt Src\YuEngine\Resource\Include\YuEngine\Resource\ResourceConstants.h Src\YuEngine\RuntimeAsset\Src\RuntimeAssetData.cpp Tests\RenderScene\RuntimeAssetDataClosedLoopTests.cpp | rg -n "\.yu(mesh|mat|tex|program|scene|anim)|suffix|fixture name|type truth|internal metadata"
```

Local result: off-scope scan has one added diagnostic string containing
`render input`, not a YuInput/UI/editor dependency; suffix scan has only test
fixture path locators plus a path-independent payload test, not type truth.

Scope accepted:

- bounded scene records support declared entity and camera counts with caller
  capacity checks before commit;
- bounded entity rows resolve mesh/material/texture/shader/animation refs by
  request file metadata, reject missing refs and duplicate world ids, sort
  deterministically, and keep path suffixes out of type truth;
- camera rows reject missing active camera and duplicate active cameras;
- bounded animation tables support declared clips/tracks/keyframes, target
  refs, linear interpolation, finite values, monotonic keyframes, target
  mismatch diagnostics, optional hash mismatch diagnostics, and sampled transform
  application through `AnimationRuntimeSampler` and `WorldTransformBridge`;
- scene staging happens before runtime mutation; failure tests keep caller
  loaded files, scene refs, cameras, entities, transforms, scene output,
  Resource registry/cache/decoded payload snapshots, Asset snapshots, and RHI
  handle snapshots unchanged;
- success path commits scene output and routes bounded loader-produced records
  through `RenderSceneRuntimeFrameBuilder`, `RenderDrawableFramePipeline`,
  RenderCore, and RHI capture.

Boundary: #60 proves the current bounded scene/animation record loader slice.
It is not a generic scene editor, editor/import/gameplay/save system, original
package parser, full production binary format, shader/program payload approval,
or final render closure.

## Focused Proof Rows

| Proof area | Minimum focused tests or equivalents | Status |
| --- | --- | --- |
| Source/cooked parser | missing schema, wrong kind, unsupported version, invalid count/size/alignment/hash, misleading suffix | PASS at `232c911`: `RuntimeAssetData_HeaderParserRejectsPartialVersionsAndNoise`, `RuntimeAssetData_SourceCookedParserReportsBoundedMetadata`, `RuntimeAssetData_SourceCookedParserRejectsInvalidTablesHashesAndDependencies`, `RuntimeAssetData_LoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation` |
| Loader transaction | preflight failure no mutation, commit failure `mutated_state`, dependency/decoded-payload intent ordering | PASS at `81c97be`: `RuntimeAssetData_LoaderRejectsMissingSchemaBeforeMutation`, `RuntimeAssetData_LoaderCommitFailureReportsMutatedState`, `RuntimeAssetData_LoaderRejectsSchemaKindAndMisleadingSuffixBeforeMutation` |
| Texture/material payload | cooked texture layout/hash/row pitch, slot resolution, invalid payload no output mutation, RHI texture cleanup | PASS at `f32ee36`: `RuntimeAssetData_CookedTexturePayloadTableValidatesLayoutHashAndRowPitch`, `RuntimeAssetData_CookedMaterialTextureSlotTableResolvesLoadedPayloads`, `RuntimeAssetData_CookedPayloadBridgeRejectsTextureFormatExtentSizeAlignmentHashWithoutMutation`, `RuntimeAssetData_CookedPayloadBridgeRejectsMissingDuplicateTypeMismatchDepsWithoutMutation`, `RuntimeAssetData_CookedMaterialSlotOverflowDoesNotMutateRenderSceneOutputs`, `RuntimeAssetData_CookedRhiPartialCreationFailureDestroysTransientHandles` |
| Shader/program payload | cooked stage bytecode, reflection/input-layout, hash/stage mismatch no mutation, module/pipeline cleanup | pending |
| Scene/animation loader | bounded N entities, capacity overflow, invalid transforms/keyframes, target mismatch, path independence, RenderScene consumption | PASS at `749e2e6`: `RuntimeAssetData_SceneLoaderRejectsInvalidEntityWithoutOutputMutation`, `RuntimeAssetData_SceneLoaderRejectsInvalidKeyframesWithoutOutputMutation`, `RuntimeAssetData_SceneAnimationLoaderLoadsBoundedNEntityScene`, `RuntimeAssetData_SceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation`, `RuntimeAssetData_SceneAnimationLoaderRejectsMissingRefsWithoutMutation`, `RuntimeAssetData_SceneAnimationLoaderRejectsInvalidRecordsWithoutMutation`, `RuntimeAssetData_SceneAnimationLoaderPathIndependentSceneAnimationDetection` |
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
