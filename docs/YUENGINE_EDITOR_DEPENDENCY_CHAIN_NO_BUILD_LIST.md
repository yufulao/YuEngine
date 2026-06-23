# YuEngine Editor Dependency Chain And No-Build-Yet List

Status: planning boundary
Owner: Architecture
Task: YuPart #40
Baseline: `origin/main@540f986`

Related documents:

- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_ANIMATION_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_STAGE0_DECISIONS.md`
- `docs/YUENGINE_UI_STAGE1_VALIDATION.md`
- `docs/YUENGINE_UI_STAGE2_VALIDATION.md`
- `docs/YUENGINE_UI_STAGE3_VALIDATION.md`

## Purpose

This document records the dependency chain that must close before Scene,
Animation, or UI editor implementation tasks are created. It also lists the
work that must stay "no build yet" until the required lower gates are accepted.

The editor direction remains:

```text
runtime data files
-> validation / import / cook / package records
-> engine preview host session
-> YuEngine runtime viewport, frame, status, and diagnostics
-> native/engine editor panels and commands
```

This document does not approve editor implementation, CMake targets, native
editor apps, preview-host code, Resource Browser code, runtime asset expansion,
or UI/Scene/Animation surface work. It is a sequencing and scope-control
document.

## Current Evidence Baseline

The Web editor cleanup baseline is `540f986`. Deprecated browser/editor-web
source, tests, and active wording have been removed or tombstoned by the Web
cleanup task group. Web, HTML/CSS, DOM, React/Vite, browser-only canvas, static
screenshots, and administrative-form pages are not active editor routes.

`YuRuntimeAsset` now proves a first smoke route for deterministic files,
validation, Resource/Asset records, decoded mesh/material/texture payload
records, RenderScene records, and RenderCore/RHI capture. The production
runtime asset contract remains open for full typed validation across file
families, decoded texture upload into material slots, shader bytecode/program
ownership, animation clip sampling from disk, and production scene loader APIs.

`EDITOR-GATE-001` started as review-only architecture guidance. The later
#YuPart task #65 authorizes only the bounded `YuPreviewHost` first slice
documented in `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`: value/session records,
RuntimeAsset graph/ref validation, bounded diagnostics, and RenderScene/
RenderCore/RHI frame or capture output. It still does not approve a native
editor app, immediate-mode fallback, local editor service process, Resource
Browser UI, Scene Editor, Animation Editor, UI Editor, or editor surface.

UI Stage 1 through Stage 3 validation documents record accepted generic UI Core,
Component Library, and UIManager runtime framework evidence. They do not
authorize UI Editor Stage 4 implementation. UI editor work still depends on
runtime UI data schemas, Resource/Asset/Cook validation, shared preview-host
output, and runtime diagnostics.

Scene and Animation editor plans are architecture plans. Scene work is not
usable editor progress until it can load model, texture, and material resources
into an engine-rendered viewport with camera controls and transform feedback.
Animation work is not usable editor progress until runtime playback, scrub,
step, keyframe/sample feedback, and event timing are visible against a runtime
preview target or an accepted headless sample output.

## Shared Dependency Chain

The shared chain is strict. Later editor work may reference earlier layers, but
it must not make earlier runtime layers depend on editor-only code.

The positive editor dependency chain is:

```text
RuntimeAsset v0
-> Resource Browser / import diagnostics
-> Preview Host
-> Scene format / viewport
-> Scene Editor
-> Animation runtime preview / editor
-> UI runtime preview / editor
-> cook / package / run smoke
```

L0/L1 runtime foundation is the prerequisite floor below this chain. It is not
itself an editor milestone.

```text
0. L0/L1 runtime foundation
1. RuntimeAssetData production contract
2. Resource Browser / import settings / cook diagnostics scope
3. Editor Runtime Preview Host implementation gate
4. Scene runtime format and viewport proof
5. Scene Editor surface
6. Animation runtime preview and editor surface
7. UI runtime preview and editor surface
8. Cook / package / run smoke for authored data
```

### 1. L0/L1 Runtime Foundation

Prerequisite runtime rows include File/VFS, Resource, Package, Asset,
RenderScene, RenderCore, RHI, World/Object/Transform/Component, Animation,
UI Core, UI Component Library, UIManager runtime, and bounded Diagnostics.

L0/L1 fixture captures, D3D/RHI samples, RenderCore fixture passes, and
standalone screenshots are prerequisites only. They must not be reported as
Scene Editor, Animation Editor, UI Editor, or Preview Host completion.

### 2. RuntimeAssetData Production Contract

The editor data spine must be disk-backed runtime data, not editor memory
objects or UI panel state.

Required before editor surfaces can claim shippable authoring:

- common header, version, bounds, dependency, hash, coordinate, status, and
  no-mutation validator vocabulary across runtime data families;
- typed mesh, material, texture, shader/program, scene, camera, and animation
  dependency records;
- File/VFS/Resource read path for source bytes and cooked/runtime records;
- Resource and Asset dependency edges;
- production scene loader output APIs;
- decoded texture payload consumption by material/RenderScene/RHI paths;
- shader bytecode/program ownership;
- animation clip loading and sampling from disk or a named blocker accepted by
  the same gate.

### 3. Resource Browser / Import / Cook Diagnostics

Editor panels need a common resource and import contract before family-specific
surfaces can be useful.

Required before resource picker panels are built:

- resource identity and type vocabulary shared by UI, Scene, Animation, and
  resource-focused preview;
- import settings records for model, texture, material, sprite, font, clip, and
  audio where relevant;
- validator and cook diagnostics for missing, stale, type-mismatched,
  unsupported, uncooked, duplicate, capacity, and budget failures;
- no hidden asset database shape copied from Unity or Unreal;
- no Game Adapter or old TouhouNewWorld package compatibility route as the
  first resource contract.

### 4. Editor Runtime Preview Host Implementation Gate

`EDITOR-GATE-001` must move from review-only guidance to an approved
implementation gate before preview-host code or targets are created.

The first implementable gate must define bounded value contracts equivalent to:

- session identity and lifecycle;
- request identity and document kind;
- frame or headless output descriptor;
- camera state;
- resource references and diagnostics;
- hit, selection, transform, playback, UI layout, and counter records;
- stale-session, small-buffer, invalid-request, and no-mutation failures;
- explicit shutdown and capacity release behavior.

The canonical scene proof remains the shared visual floor: cube, cylinder, and
cone from runtime data; deterministic placement; per-object rotation; one
material with at least three texture inputs; orbit camera; bounded captured
frame set; and exact missing-layer diagnostics when a layer is absent.

### 5. Family Runtime Documents And Validators

Family-specific editor work starts with runtime documents and validators, not
panels.

Scene must start from scene document records for object identity, transform,
component/resource binding, assembly manifests, editor-only sidecars, duplicate
and missing-reference validation, decoded restore plan, apply-time proof, and
RenderScene/AudioScene projections. Scene viewport proof must precede the
Scene Editor surface.

Animation must start from animation document records for asset identity, import
settings, clip metadata, track descriptors, keyframes/samples, event markers,
playback commands, optional state-preview records, editor-only sidecars, and
runtime sample/event diagnostics. Animation runtime preview must follow the
Scene viewport/resource proof and precede the Animation Editor surface.

UI must start from UI runtime data for schema/version, layout ID, root node,
deterministic node array, anchors, offsets, pivot, margin, padding, size policy,
style refs, resource refs, stable event/binding keys, editor-only sidecars, and
runtime layout parity fixtures against the YuEngine solver. UI runtime preview
must follow the shared preview-host/resource route and precede the UI Editor
surface.

### 6. Preview-Host Integration

Each family may integrate with the shared preview host only after its runtime
data and validator route exists.

Scene preview needs engine-rendered or accepted headless scene output, camera
state, resource resolution, selection, hit, transform feedback, and diagnostics.

Animation preview needs play, pause, scrub, step, runtime sample feedback,
event timing, and visible target feedback or accepted headless sample output.

UI preview needs YuEngine UI runtime layout/render output, resource diagnostics
for sprites/textures/fonts/atlases/materials, coordinate adapter records, and
runtime-layout parity tests. HTML/CSS layout is not an authority.

### 7. Native/Engine Editor Panels

Editor panels come after runtime data, validation, Resource Browser scope, and
preview-host integration.

Panels may own hierarchy, inspector, timeline, resource picker, style/theme
controls, layout viewport interaction, command routing, selection state,
shortcuts, undo/redo, and editor-only workflow. They must write runtime data or
editor-only sidecar data and must not become shipped runtime dependencies.

### 8. Cook / Package / Run Smoke

Editor-authored data is not shippable until the same runtime data validates,
cooks, packages, loads, previews, and runs through approved YuEngine paths.

Cook/package/run smoke must consume the authored data, not a parallel fixture
or editor-only export.

## Per-Family First Allowed Landing Batches

These batches are allowed only after the shared prerequisite for that row is
accepted. They are listed so future task creation can stay parallel and narrow.

| Family | First batch after prerequisites | Must not include |
| --- | --- | --- |
| Scene | document header, object identity, transform records, editor-only sidecar split, duplicate/missing-id validation | native scene editor app, Web/canvas viewport, transform gizmo implementation, model/material/texture preview claims |
| Animation | document header, clip metadata, track descriptors, event markers, editor-only sidecar split | gameplay FSM, state-machine editor, timeline-only UI, playback claims without runtime target feedback |
| UI | UI file schema, node tree/layout/style/resource records, editor-only sidecar split, duplicate/missing-ref validation, coordinate spec | HTML/CSS preview, native app shell, game-specific windows, normal editor iteration requiring CMake rebuilds |
| Resource Browser | resource identity/type records, import settings vocabulary, diagnostics records, cook command boundary | asset database clone, old package parser, Game Adapter flow, UI-only resource picker with no runtime validation |
| Preview Host | session lifecycle, frame/headless descriptor, camera state, resource diagnostics, selection/hit records, canonical scene proof contract | editor panels, UI runtime implementation, Scene runtime expansion, Animation runtime expansion, Web compatibility layer |

## No-Build-Yet List

Do not create implementation tasks, CMake targets, source directories, tests, or
sample binaries for the following until an explicit later gate approves them.

### Shared Editor Infrastructure

- no `EditorRuntimePreviewHost` implementation target yet; task #65 adds only
  the lower `YuPreviewHost` engine value/session target, not an editor app or
  local editor service;
- no local editor service process or daemon target yet;
- no native editor application target yet;
- no immediate-mode native editor fallback target yet;
- no editor plugin, extension, or launcher target yet;
- no shared editor panel framework target yet;
- no Web compatibility, browser shell, React/Vite, HTML/CSS, DOM, or canvas
  preview target under a new name;
- no editor heartbeat, polling, or preview status path that enters durable
  runtime event streams.
- no marketplace, plugin ecosystem, extension store, package marketplace, or
  third-party editor plugin API surface yet.

### RuntimeAssetData Expansion

- no new production mesh/material/texture/shader/scene/animation file-family
  expansion from editor needs until the RuntimeAssetData production contract is
  explicitly scoped;
- no editor-owned loader that bypasses File/VFS/Resource;
- no fixture-struct injection into RenderScene as asset-loading proof;
- no CPU image, screenshot, report, or viewer artifact accepted as final proof;
- no original-game package or parser route as the first runtime data contract.
- no cloud cook, remote cook, distributed build farm, or background cook
  service yet.

### Resource Browser And Import

- no Resource Browser UI target yet;
- no import settings editor panel yet;
- no cook/pack diagnostics UI target yet;
- no Unity/Unreal asset database shape copied into YuEngine;
- no game-specific resource browser categories;
- no resource picker that writes editor-only paths as runtime references.
- no source-control suite, changelist UI, branch UI, lock manager, diff/merge
  tool, or Perforce/Git workflow integration yet.

### Scene Editor

- no Scene Editor native/engine surface target yet;
- no scene hierarchy, inspector, viewport, transform gizmo, or drag/drop
  implementation yet;
- no editor-only scene graph as source of truth;
- no Web/canvas/static scene viewport fallback;
- no usable Scene Editor claim before model, texture, material, camera,
  transform feedback, and engine-rendered viewport or accepted headless preview
  are available through the shared preview host;
- no bypass of decoded restore plan or apply-time proof for scene data.
- no world partition, streaming landscape, foliage painter, terrain system,
  Nanite-like virtualized geometry system, broad world streaming, or editor
  large-world management system yet.

### Animation Editor

- no Animation Editor native/engine surface target yet;
- no timeline, graph, state machine, scrubber, or event editor implementation
  yet;
- no Unity Animator, Unreal Sequencer, montage, blend tree, or gameplay FSM
  clone;
- no visual scripting surface, node graph gameplay authoring, behavior tree
  editor, or blueprint-like runtime graph yet;
- no timeline-only proof without runtime clip/player/sample/event output;
- no usable Animation Editor claim before play, pause, scrub, step,
  keyframe/sample feedback, and event timing exist against a runtime preview
  target or accepted headless sample output.

### UI Editor

- no UI Editor native/engine surface target yet;
- no hierarchy, inspector, template browser, theme editor, state-preview panel,
  coordinate adapter, or engine UI viewport implementation yet;
- no HTML/CSS, DOM, browser canvas, or static screenshot preview under a
  different name;
- no game-specific window migration or game-window validation samples;
- no editor-only module dependency from shipped UI runtime;
- no usable UI Editor claim before YuEngine UI runtime viewport/frame/status
  and diagnostics are available through the shared preview host;
- no normal hierarchy/inspector/template/theme/state-preview iteration that
  requires CMake or C++ rebuilds.
- no general material graph, shader graph, node-based material editor, or
  renderer plugin authoring surface yet.

## Task-Creation Rules

Before any future editor implementation task is created, the task must state:

- approved gate and document baseline;
- owned layer and explicitly excluded layers;
- runtime data source of truth;
- dependency direction;
- allowed references and what is not copied;
- bounded value records and capacity limits;
- validator/cook/package/run evidence route;
- focused tests and off-scope dependency scans;
- exact blocker name for any missing lower runtime layer;
- reason the task does not resurrect Web, native-app fallback, game-specific
  migration, or editor-only runtime dependency.

If the task cannot state those items, it remains documentation or review work.

## Allowed Work Before Build Approval

The following work can continue without creating editor implementation targets:

- architecture review and task sequencing;
- dependency matrices and no-build lists;
- RuntimeAssetData production-gap closure planning;
- preview-host implementation-gate review;
- Resource Browser/import/cook diagnostics scope review;
- Scene, Animation, and UI runtime document schema review;
- audits that prove no deprecated Web route, native app fallback, or
  game-specific migration route is being revived.

## Verification For This Document

This document is docs-only. Required verification is:

```powershell
git diff --check
```

No configure, build, CTest, preview host, or runtime sample command is required
for this planning boundary unless a reviewer expands the scope.
