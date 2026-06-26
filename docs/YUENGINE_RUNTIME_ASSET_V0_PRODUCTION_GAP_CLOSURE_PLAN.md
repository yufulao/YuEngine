# YuEngine RuntimeAsset v0 Production-Gap Closure Plan

Status: RAV0 floors implemented; RAV1 production contract gate in review
Owner: Architecture
Task: #36 baseline; #50 RAV1 production contract amendment
Related plan: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
Format policy and validator vocabulary: `docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`
Loader transaction plan: `docs/YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md`
Payload bridge RHI route plan: `docs/YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md`
RAV1 evidence matrix: `docs/YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md`

## Purpose

This plan closes the gap between the implemented `YuRuntimeAsset` smoke path and
a production-ready RuntimeAsset v0 data spine.

It is not an implementation approval. It defines the work that must close before
downstream editor, preview-host, resource-browser, import/cook diagnostics, or
scene/animation/UI editor tasks may treat RuntimeAsset v0 as stable.

## Audited Surface

This plan audits:

- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
- `Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h`
- `Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp`
- `Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp`

Current implementation surface is intentionally compact: one public
`RuntimeAssetData` header/source pair and one RenderScene closed-loop test file.

## Already Proven Smoke

The current implementation proves these useful floors:

| Area | Proven floor |
| --- | --- |
| Disk source | deterministic generated mesh/material/texture/shader/scene/animation files are written and re-read |
| Header and bounds validation | unsupported version, invalid mesh bounds, and scene missing/duplicate dependency smoke fail without output mutation |
| File path | graph load reads through `MountTable` rather than only C++ in-memory structs |
| Runtime records | Resource and Asset records are registered for scene plus generated asset families |
| Dependency edges | scene dependencies are added to both Resource and Asset graphs |
| Cook/decode smoke | cache payloads and deterministic decoded payload records are stored for mesh/material/texture families |
| Render path | loaded handles feed RenderScene records and RenderCore/RHI captures cube/cylinder/cone |
| Oracle guard | CPU helper output is guarded behind runtime capture |
| Upper-layer guard | editor, rejected editor route, UI, input, and GDI viewer dependencies are excluded from the smoke |

These floors are enough to reject pure test-struct and helper-image claims. They
are not enough to call RuntimeAsset v0 production-ready.

## Format Naming And Identification Policy

The current `.yumesh`, `.yumat`, `.yutex`, `.yuprogram`, `.yuscene`, and
`.yuanim` names are smoke-fixture names only. They are not a production format
naming strategy, and production RuntimeAsset v0 must not depend on filename
extensions to identify asset type.

Source and authoring-side files should prefer AI- and human-readable
schema-shaped text or manifests where useful for single-team iteration speed.
Type identification must come from internal metadata such as `kind`, `version`,
`schema`, and dependency table records, not from suffixes.

Runtime, cook, and export output should generate high-performance binary data
for fast loading and validation. Binary runtime files must carry internal magic,
version, kind, hash, dependency, and table metadata sufficient to validate and
load without trusting external names.

Do not trade YuEngine format cleanliness for external ecosystem compatibility,
plugin marketplace conventions, or commercial-engine import format parity. The
priority is single-team development efficiency on the source side and YuEngine
runtime performance on the cooked/runtime side.

## RAV0 Production Floors And Remaining RAV1 Gate Work

RAV0 implementation tasks have now proven the first production floors for:

- shared format/status vocabulary and suffix-free identity checks;
- typed mesh/material/texture validators;
- shader/program, scene/camera, and animation dependency validators;
- decoded texture payloads driving material slot bridge tests;
- shader bytecode/program ownership through RHI shader module and pipeline
  creation tests;
- disk animation sampling feeding scene transforms;
- deterministic staged scene loader output and scene loader no-mutation failures.

Those floors close the original "smoke only" blockers. The remaining RAV1-A
work is review-only docs/gate work: freeze the source/cooked artifact contract,
the suffix-free cook/load/render route, the exact #41 status vocabulary usage,
and the evidence boundary before any follow-up implementation task treats
RuntimeAsset v0 as stable.

## Production Contract Requirements

### A. Typed Validators For Every File Family

RuntimeAsset v0 keeps typed validators for every approved family:

- mesh
- material
- texture descriptor and payload reference
- shader/program descriptor
- scene
- camera if kept as separate data rather than embedded scene data
- animation clip and sampled transform refs

Each validator must enforce common file rules: magic/version, byte order, count
bounds, byte-size bounds, string/path bounds, alignment, identity/hash/size,
typed dependency refs, coordinate/units/handedness/winding/UV origin, and explicit
statuses for invalid header, unsupported version, invalid size/count, invalid
dependency, duplicate id, missing dependency, type mismatch, hash mismatch,
unsupported field value, output capacity exceeded, and budget exceeded.
Validators must use internal kind/version/schema metadata and must not infer
file family from `.yu*` or any other filename suffix.

Acceptance shape:

- one named no-mutation test per file family for invalid header/version/bounds;
- one named no-mutation test per dependency-bearing family for
  missing/duplicate/type-mismatch refs;
- one output-capacity/budget no-mutation test covering loader output buffers;
- no validator may mutate Resource, Asset, RenderScene, RenderCore, RHI, or test
  side-channel state before validation succeeds.

### B. Decoded Texture Payload To RHI Material Slots

RAV0 #44 proves that loaded texture metadata can drive decoded payload upload
through `ResourceDecodedTextureBridge`, `AssetManager::MarkTextureReady`, and
`RenderSceneRuntimeMaterialTextureSlot`. RuntimeAsset v0 production still needs
cooked texture payload ownership instead of fixture-shaped payload assumptions.

RAV1-C defines the production bridge route in
`docs/YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md`. The RAV0 #44
implementation is accepted only as a decoded-payload smoke floor: production
still needs cooked texture payload records carrying descriptor metadata, row
pitch, payload size/hash, color-space policy, and Resource decoded payload
identity before RHI upload.

Acceptance shape:

- loaded material slots resolve texture refs to decoded texture payload records;
- RHI texture creation/update uses decoded payload bytes and descriptor metadata;
- missing, corrupt, wrong-format, or over-budget texture payloads return explicit
  statuses before RenderScene output mutation;
- tests must prove that direct test-created texture handles are not sufficient
  RuntimeAsset acceptance.

### C. Shader / Program Bytecode Ownership

RAV0 #45 proves that `RuntimeAssetLoadedShaderProgramData` can feed
`BuildRuntimeAssetShaderProgramPipeline`, creating RHI shader modules and a
pipeline from loaded/decoded program data instead of local test byte arrays.
RuntimeAsset v0 production still needs cooked shader/program payload ownership
instead of `bytecode:` source-text assumptions.

RAV1-C keeps the RAV0 #45 bridge as a smoke floor only. Production shader/program
records still need cooked stage payload ownership, bytecode size/hash/alignment,
minimal reflection, input-layout, texture/sampler slot counts, and cleanup rules
for partial RHI shader module or pipeline creation.

Acceptance shape:

- program descriptor carries stage refs, bytecode payload hash/size, entry
  semantics, input layout, constant ranges, texture slots, and required
  semantics;
- decoded shader payload records are consumed when creating RHI shader modules;
- pipeline creation consumes loaded program/input-layout data rather than local
  test constants;
- invalid stage refs, mismatched input layout, missing bytecode, hash mismatch,
  and capacity overflow fail with explicit no-mutation statuses.

### D. Disk Animation Sampling

Current smoke registers an animation file and scene dependency, but the loaded
graph does not yet sample clip keyframes from disk data or apply those samples to
the final scene transform path.

RuntimeAsset v0 must either close animation sampling or keep animation as a
named same-gate blocker. It must not silently replace missing animation data with
hardcoded per-frame math.

Acceptance shape:

- animation file validator parses clip id, track id, target entity/transform ref,
  sample rate, keyframe ranges, interpolation mode, and bounds;
- loaded animation data is sampled through the approved Animation runtime
  sampler;
- sampled transforms are applied to the scene frame used by RenderScene;
- invalid time ranges, target mismatches, unsupported interpolation, empty
  keyframes, and output capacity overflow fail without partial scene mutation.

### E. Production Scene Loader Output API

Current closed-loop tests build the final RenderScene frame through proof-route
helpers after loading the graph. RuntimeAsset v0 needs a production scene loader
output API that owns the handoff between loaded runtime data and RenderScene
records.

Acceptance shape:

- public output records expose loaded scene id, entities, transforms, mesh refs,
  material refs, texture refs, shader/program refs, camera refs, animation refs,
  diagnostics, and explicit capacity counts;
- loader output is deterministic and budgeted;
- RenderScene consumes loader output records without test-private fixture
  bridges;
- missing family, invalid ref, missing decoded payload, missing shader program,
  missing camera, missing animation sample, and output-capacity failures report
  exact layer/status before runtime output mutation.

## Closure Phases

| Phase | Work | Parallelization |
| --- | --- | --- |
| RAV0-0 | Freeze common header/status/bounds/dependency vocabulary | single architecture slice |
| RAV0-1 | Typed family validators | parallel by mesh/material/texture, shader/program, scene/camera, animation |
| RAV0-2 | Cook/decode payload bridges | parallel texture payload -> RHI material slots, shader bytecode -> RHI program, animation clip -> sampler |
| RAV0-3 | Production scene loader output API | single integration slice after RAV0-1 and relevant RAV0-2 outputs |
| RAV0-4 | End-to-end closed-loop verification | integration slice proving generated disk files -> validator/cook/load -> RenderScene/RenderCore/RHI capture |
| RAV1-A | Production RuntimeAsset v0 contract and cook/load/render gate | docs-only architecture slice; no runtime implementation approval |
| RAV1-B | Suffix-free loader transaction and no-mutation API plan | design slice consuming this contract |
| RAV1-C | Cooked texture/material/shader payload bridge plan to RHI route | design slice consuming this contract |
| RAV1-D | Bounded scene/animation record loader plan | design slice consuming this contract |
| RAV1-E | Evidence matrix and acceptance commands | review/evidence slice before implementation authorization |
| RAV2-A | Import/cook command contract and deterministic disk fixture generator | first engine-owned command/API slice; writes source+cooked disk fixtures through File/VFS and exposes descriptors for RuntimeAsset graph load |

Every implementation task must restate that editor, rejected editor route, UI, Game Adapter,
original package parser, and external authoring bridge work are out of scope.
RAV1-A is not an implementation task; it only records the contract and gate that
later slices must obey.

RAV2-A is documented in
`docs/YUENGINE_RUNTIME_ASSET_V0_IMPORT_COOK_COMMAND_CONTRACT.md`. It is not
Resource Browser UI, Preview Host, original package parsing, Unity/UE importing,
or final render closure.

## Downstream Blockers

### #38 Engine Preview Host MVP Gate

Blocked until these RuntimeAsset v0 items close:

- decoded texture payload to RHI material slots;
- shader/program bytecode ownership;
- production scene loader output API.

The preview host may define how it will consume RuntimeAsset output, but it must
not count preview-host usability until loaded RuntimeAsset data can drive the
runtime preview path without test-private bridges.

### #39 Resource Browser + Import/Cook/Diagnostics Scope

Blocked until these RuntimeAsset v0 items close:

- common typed validator/status vocabulary;
- per-family no-mutation validator contract;
- cook/decode ownership boundaries for texture, shader/program, and animation.

Resource Browser/import/cook diagnostics may classify UX and diagnostic surfaces,
but they must not invent RuntimeAsset format semantics or treat editor/import UI
as the runtime data source of truth.

### #40 Scene/Animation/UI Editor Dependency Chain

Blocked until these RuntimeAsset v0 items close or are explicitly named as
remaining same-gate blockers:

- disk animation sampling;
- production scene loader output API;
- texture/material/shader bridges consumed by RenderScene.

The editor dependency chain must list these as no-build-yet dependencies. It must
not create Scene, Animation, or UI editor implementation tasks that depend on
RuntimeAsset v0 before this plan's exit criteria are met.

## Hard Boundaries

Do not include in RuntimeAsset v0:

- editor preview host implementation;
- rejected editor route editor, browser canvas, UI editor, native editor shell, or viewport overlay;
- Game Adapter behavior;
- old TouhouNewWorld package parser or original package compatibility;
- external ecosystem, plugin-marketplace, or commercial-engine format
  compatibility constraints;
- external DCC/authoring bridge, hot reload, asset database UX, or remote cook;
- screenshots, reports, CPU PPM artifacts, GDI/software viewers, sleeps, or
  manual inspection as acceptance.

Original package/resource facts may inform later evidence after a separate
package/parser gate. They must not define RuntimeAsset v0 API shape.

## Exit Criteria

RuntimeAsset v0 RAV1 implementation split is ready only when:

1. the RAV1-A production contract and paired gate are committed and accepted as
   review-only docs/gate output;
2. common family validator/status vocabulary remains the #41
   `RuntimeAssetDataStatus` vocabulary and no parallel family status enum is
   introduced;
3. all file-family validators have named no-mutation tests or approved
   equivalents in the RAV1 evidence matrix;
4. texture decoded payloads drive RHI material slots from loaded data;
5. shader/program bytecode records drive RHI shader module and pipeline creation;
6. animation file data is sampled through the Animation runtime sampler, or a
   same-gate blocker remains explicit;
7. production scene loader output records feed RenderScene without test-private
   bridges;
8. focused RuntimeAssetData tests and full `windows-fast-gate` pass;
9. dependency scans prove no editor, rejected editor route, UI, Game Adapter, original package
   parser, external authoring bridge, report, screenshot, GDI viewer, or helper
   image path is part of acceptance.
