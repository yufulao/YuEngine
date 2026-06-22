# YuEngine RuntimeAsset v0 Production-Gap Closure Plan

Status: planning / gate dependency
Owner: Architecture
Task: #36
Related plan: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
Format policy and validator vocabulary: `docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`

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
| Upper-layer guard | editor, Web, UI, input, and GDI viewer dependencies are excluded from the smoke |

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

## Open Production Gaps

### A. Typed Validators For Every File Family

Current validation is header and selected string-token smoke. RuntimeAsset v0
must add typed validators for every approved family:

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

Current smoke stores texture decoded payload records, but RenderScene test
material slots still create RHI texture handles from deterministic fixture bytes.
RuntimeAsset v0 must bridge decoded texture payload ownership into the material
slot path consumed by RenderScene/RenderCore/RHI.

Acceptance shape:

- loaded material slots resolve texture refs to decoded texture payload records;
- RHI texture creation/update uses decoded payload bytes and descriptor metadata;
- missing, corrupt, wrong-format, or over-budget texture payloads return explicit
  statuses before RenderScene output mutation;
- tests must prove that direct test-created texture handles are not sufficient
  RuntimeAsset acceptance.

### C. Shader / Program Bytecode Ownership

Current shader/program descriptor smoke proves a referenced file exists and is in
the dependency graph. RHI shader modules and pipelines still use fixture byte
arrays created by the test path.

RuntimeAsset v0 must own shader/program bytecode records enough for the runtime
path to build RHI shader modules and pipelines from loaded data.

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

Every implementation task must restate that editor, Web, UI, Game Adapter,
original package parser, and external authoring bridge work are out of scope.

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
- Web editor, browser canvas, UI editor, native editor shell, or viewport overlay;
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

RuntimeAsset v0 production-gap closure is ready for implementation split only
when:

1. common family validator/status vocabulary is accepted;
2. all file-family validators have named no-mutation tests;
3. texture decoded payloads drive RHI material slots from loaded data;
4. shader/program bytecode records drive RHI shader module and pipeline creation;
5. animation file data is sampled through the Animation runtime sampler, or a
   same-gate blocker remains explicit;
6. production scene loader output records feed RenderScene without test-private
   bridges;
7. focused RuntimeAssetData tests and full `windows-fast-gate` pass;
8. dependency scans prove no editor, Web, UI, Game Adapter, original package
   parser, external authoring bridge, report, screenshot, GDI viewer, or helper
   image path is part of acceptance.
