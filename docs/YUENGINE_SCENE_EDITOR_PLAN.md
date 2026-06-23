# YuEngine Scene Editor Plan

Status: architecture plan
Requested: 2026-06-21
Owner: Architecture
Scope: Scene Editor, scene runtime data, scene validation, runtime preview

Related:

- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
- `docs/gates/EDITOR_GATE_002_SCENE_EDITOR_DEPENDENCY_GATE.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV5_SCENE_DOCUMENT_BRIDGE.md`
- `docs/YUENGINE_SCENE_EDITOR_RAV5_HIERARCHY_INSPECTOR_SURFACE.md`
- `docs/YUENGINE_SCENE_EDITOR_RAV5_TRANSFORM_COMMAND_UNDO_REDO.md`
- `docs/YUENGINE_RAV5_EDITOR_SURFACE_REVIEW_EVIDENCE.md`
- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/YUENGINE_PHASE3_ARCHITECTURE_QUEUE.md`
- `docs/gates/P3_GATE_017_WORLD_SCENE_ASSEMBLY_SNAPSHOT_RESTORE_COORDINATOR.md`
- `docs/gates/P3_GATE_018_WORLD_SCENE_ASSEMBLY_MANIFEST_STREAM_BRIDGE.md`
- `docs/gates/P3_GATE_021_WORLD_SCENE_DECODED_RESTORE_PLAN_BRIDGE.md`
- `docs/gates/P3_GATE_022_WORLD_SCENE_APPLY_TIME_RESTORE_PROOF_BRIDGE.md`

## 1. Decision

Scene Editor work must be a narrow scene-assembly data editor and validator
with a real engine viewport. It is not a full Unity or Unreal editor clone, but
the minimum usable floor is a Unity-like operational editor, not a 2D web mock.

The useful first direction is:

```text
runtime-loadable scene records
-> native/engine editor surface
-> local scene editor service
-> cook/package/resource validation
-> engine preview host viewport and diagnostics
```

The editor edits data that the runtime can load. The editor does not define the
runtime behavior.

Scene Editor must share the production loop with UI Editor and Animation
Editor:

- Resource Browser and Import Settings
- stable runtime data formats
- cook and package validation
- runtime preview and diagnostics
- build, run, and package checks

An editor report that only proves hierarchy drawing, a viewport mock, or JSON
editing is incomplete unless the data can pass this loop.

Scene Editor is not usable until it can show engine-rendered scene content:

- camera and viewport controls
- object selection
- translate/rotate/scale gizmo feedback
- model loading entry
- texture and material loading entry
- resource binding diagnostics
- runtime scene data that can be cooked, packaged, and loaded

The L0/L1 execution plan, RHI capture-byte fixtures, RenderCore fixture passes,
RenderScene packet-value tests, and isolated sample screenshots are lower-level
prerequisites only. They do not count as Scene Editor viewport completion.

## 2. Non-Goals

Scene Editor does not own:

- a general editor plugin ecosystem
- a native C++ editor app or immediate-mode fallback
- a 2D canvas or HTML page as core scene preview
- a copied Unity Scene/GameObject/Component editor model
- a copied Unreal World/Actor/Component editor model
- gameplay scene flow, quest triggers, player spawn policy, save-slot policy, or
  old project scene managers
- old TouhouNewWorld scene compatibility
- object construction policy beyond approved runtime data contracts
- resource loading expansion beyond approved Resource/Package/File gates
- material graph editing, shader editing, animation editing, UI editing, audio
  graph editing, physics authoring, or scripting IDE behavior

## 3. Source Of Truth

The runtime source of truth is YuEngine scene data, not editor object state.

The first scene document family should be based on existing runtime contracts:

- `WorldObjectId`
- world object identity restore records
- world transform restore records
- component attachment records
- component-resource binding records
- scene assembly manifest stream records
- decoded scene restore plan records
- apply-time restore proof records
- RenderScene projection records where render preview is enabled
- AudioScene projection records where audio preview is enabled

The editor may keep selection, camera, gizmo, grid, foldout, and panel state,
but this state must be editor-only and must not be exported as runtime scene
data.

## 4. Reference Inputs

Reference use is constrained to responsibility boundaries.

| Reference | Use | Not copied |
| --- | --- | --- |
| YuEngine `WorldScene*` gates | scene manifest stream, decoded restore plan, no-mutation proof, active restore separation | editor convenience state as runtime state |
| YuEngine `WorldTransformBridge` | bounded transform records and status handling | transform hierarchy or scene graph behavior not yet gated |
| YuEngine component attachment/resource binding bridges | generic component/resource sidecar records | component payload lifecycle or gameplay component logic |
| YuEngine `Resource`, `Package`, and `File` gates | resource identity, package load-plan, path/load validation | package expansion or old game package compatibility |
| YuEngine `RenderScene` and `AudioScene` | preview projections from runtime records | editor-only render/audio behavior ownership |
| YuEngine editor preview host | engine-rendered viewport, camera, transform gizmo feedback, resource diagnostics | 2D canvas mock or static screenshot as preview proof |
| old `SceneManager.cs` | high-level need for scene id, loaded scene tracking, active scene switching, and scene change notification | Unity Addressables, Unity scene APIs, config table dependency, EventManager behavior |
| Unity Scene view | hierarchy, inspector, transform editing, gizmo workflow vocabulary | Unity API shape, prefab/asset database model, serialized object model |
| Unreal level/world tooling | separation of world data, actors/components, asset refs, viewport diagnostics | Unreal API shape, actor lifecycle, editor extensibility model |

Each implementation task must record:

```text
Reference used:
Borrowed behavior:
Not copied:
YuEngine-specific acceptance:
```

## 5. Layer Model

### 5.1 Runtime Scene Data

Owns:

- schema version and document identity
- ordered object records
- transform records
- component attachment records
- component-resource binding records
- resource references by typed handles or cook-resolved logical keys
- explicit status and validation records
- deterministic stream order

Does not own:

- editor camera, selection, foldout, grid, gizmo, or undo stack
- old game scene ids as engine API shape
- config table lookup
- gameplay event dispatch
- live asset loading policy

### 5.2 Native/Engine Editor Surface

Owns:

- scene hierarchy panel
- scene viewport shell, overlays, and command routing
- transform inspector
- component attachment inspector
- resource picker integration
- validation result panel
- undo/redo command history
- selection, gizmo, snapping, grid, and viewport camera state
- editor-only templates for authoring common record groups

The scene image must come from the engine preview host. Editor overlays may
draw selection/gizmo guides, but they must not fake the scene viewport with
forbidden HTML/CSS, a browser-only canvas, or static screenshots.

### 5.3 Local Scene Editor Service

Owns backend support only:

- load/save runtime scene documents
- schema and version migration
- duplicate id and reference validation
- resource reference validation
- cook/package validation calls
- runtime preview session control
- local editor-host API or IPC contract

The service must not encode frontend panels, hierarchy composition, gizmo
behavior, browser panels, deprecated Web workflow, or editor templates in C++.

### 5.4 Runtime Preview

Preview uses the real runtime path. It is mandatory for usable-editor
acceptance:

```text
scene document
-> validate
-> decoded restore plan
-> apply-time restore proof
-> controlled runtime preview world
-> model/texture/material resource resolution
-> camera and RenderScene/AudioScene projections
-> diagnostics back to the editor host
```

The editor viewport may draw overlays, selection boxes, handles, grid, and labels.
Those overlays are never runtime scene data.

## 6. Execution Plan

### Stage 0: Boundary Freeze

Goal: prevent scene editor work from becoming a full editor ecosystem or a
forbidden Web mock.

Required:

- document Scene Editor as a runtime-data editor, not a copied Unity/Unreal
  editor
- forbid a native C++ editor shell fallback
- forbid Web, 2D canvas, HTML/CSS, and static screenshot preview as core
  acceptance
- identify exact runtime scene records that are editable
- identify editor-only state that must not export
- align Scene Editor with shared Resource Browser, cook/package validator, and
  runtime preview loop

Done when:

- this plan is accepted
- no native scene editor app target is introduced
- no game-specific scene flow appears in engine scope

### Stage 1: Scene Document Schema

Goal: define stable runtime-loadable scene documents.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| SE-S1-001 | Scene document header | schema version, document id, deterministic document hash |
| SE-S1-002 | Object identity records | ordered object records map to `WorldObjectId` restore inputs |
| SE-S1-003 | Transform records | position/rotation/scale values map to transform restore inputs |
| SE-S1-004 | Component attachment records | component type/slot tuples map to attachment restore inputs |
| SE-S1-005 | Component-resource binding records | typed resource refs map to binding restore inputs |
| SE-S1-006 | Scene manifest stream export | deterministic stream through existing YuSerialize boundaries |
| SE-S1-007 | Scene import validation | malformed stream rejects without partial editor/runtime mutation |
| SE-S1-008 | Editor-only sidecar | selection, viewport, foldout, grid, and gizmo state kept separate |

### Stage 2: Scene Validation And Cook Bridge

Goal: make the editor prove shippable data before runtime preview.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| SE-S2-001 | Duplicate/id validation | duplicate object/component/resource tuples produce explicit diagnostics |
| SE-S2-002 | Resource reference validation | missing/stale/type-mismatch resources rejected before preview |
| SE-S2-003 | Package validation bridge | scene dependencies are checked against package/load-plan data |
| SE-S2-004 | Cross-family restore plan | object/transform/component/resource records validate as one transaction |
| SE-S2-005 | Apply-time proof route | preview requires no-mutation proof before active restore |
| SE-S2-006 | Cook output manifest | editor can emit a cook-ready scene manifest and dependency list |

### Stage 3: Native/Engine Scene Editor Surface

Goal: build the authoring surface around runtime records and an engine-backed
viewport.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| SE-EW-001 | Scene hierarchy | create, delete, duplicate, reorder, and select runtime object records |
| SE-EW-002 | Transform inspector | edits runtime transform fields, not CSS or editor-only placement |
| SE-EW-003 | Component inspector | edits generic component attachment/resource binding records only |
| SE-EW-004 | Resource picker | uses shared Resource Browser and typed Import Settings |
| SE-EW-005 | Engine viewport bridge | viewport image/status comes from engine preview host |
| SE-EW-006 | Viewport overlay | selection, gizmo, grid, and labels are editor overlays only |
| SE-EW-007 | Camera and gizmo commands | orbit/pan/zoom and translate/rotate/scale commands update preview host/runtime data |
| SE-EW-008 | Model/material/texture entry | mesh/material/texture refs can be selected through Resource Browser and previewed |
| SE-EW-009 | Undo/redo commands | command log mutates runtime document data deterministically |
| SE-EW-010 | Validation panel | shows schema, resource, package, restore-plan, and preview diagnostics |
| SE-EW-011 | Editor surface test route | editor UI/data tests prove runtime-data edits without deprecated Web acceptance |

### Stage 4: Runtime Preview And Diagnostics

Goal: close the loop from edited data to runtime behavior.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| SE-PV-001 | Preview session protocol | editor host can start, update, and stop a runtime preview session |
| SE-PV-002 | Headless restore preview | scene data passes decoded plan and apply-time proof before mutation |
| SE-PV-003 | Camera viewport preview | camera state produces engine-rendered frame/status output |
| SE-PV-004 | Model/material/texture preview | renderable records load resources and show material/texture diagnostics |
| SE-PV-005 | RenderScene preview | renderable records produce preview frame/status through runtime path |
| SE-PV-006 | AudioScene preview | audio records produce preview status through runtime path when present |
| SE-PV-007 | Canonical visual scene proof | fixed-seed cube/cylinder/cone scene with shared three-texture material, per-object rotation, orbit camera, and bounded captured frame set comes from preview host |
| SE-PV-008 | Diagnostics snapshot | object count, component count, resource refs, draw/audio counts, failures |
| SE-PV-009 | Build/run/package check | sample scene enters package/build validation path |

### Stage 5: Product-Layer Handoff

Goal: allow a future project layer to use scene data without making the editor
own gameplay.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| SE-L2-001 | Scene load request contract | product layer can request a cooked scene by stable id/key |
| SE-L2-002 | Scene activation status | active scene state is runtime-owned and explicitly reported |
| SE-L2-003 | Scene change event bridge | future product event flow is a separate adapter, not editor scope |
| SE-L2-004 | Save/load handoff | save-slot policy remains product-layer work, not Scene Editor work |

## 7. Hard Blocks

These are blocking violations:

- adding game-specific scene names, quest flows, player spawn flows, or save
  policy into Scene Editor scope
- using old project scene managers as runtime API shape
- adding a native scene editor app or immediate-mode fallback
- accepting deprecated Web, 2D canvas, HTML/CSS forms, or static screenshots as scene
  preview
- accepting L0/L1 completion, RHI fixture captures, RenderCore fixture passes,
  RenderScene packet values, or isolated sample screenshots as Scene Editor
  viewport proof
- calling Scene Editor usable before model/texture/material loading entry,
  camera controls, transform gizmo, and engine viewport exist
- making the runtime depend on deprecated Web editor code
- making editor-only selection/camera/gizmo state part of runtime scene data
- bypassing scene restore plan and apply-time proof for preview
- making RenderScene, AudioScene, Resource, Package, or File expand outside
  their approved gates
- validating editor progress only by drawing a hierarchy or viewport mock
- using old game package compatibility as first-round acceptance

## 8. Verification

Documentation-only changes require:

```powershell
git diff --check
```

Runtime or service changes require:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

Focused future routes should include:

```powershell
ctest --preset windows-fast-gate -R "(WorldScene|RenderScene|AudioScene|Resource|Package)" --output-on-failure
```

Deprecated Web editor commands are not active acceptance routes. Editor surface
tests must prove runtime-data edits and preview-host integration.

## 9. Completion Definition

Scene Editor first round is complete only when:

- scene document schema is runtime-loadable
- editor-only state is separated from runtime data
- Resource Browser and Import Settings are shared
- engine preview host provides camera viewport frame/status output
- model, texture, and material resource references can be loaded or diagnosed
- the canonical cube/cylinder/cone visual scene proof passes through the
  preview host, or the exact missing runtime layer is documented as a blocker
- cook/package validation reports dependency correctness
- decoded restore plan and apply-time proof gate runtime preview
- runtime preview returns RenderScene/AudioScene/diagnostic status through
  engine paths
- build/run/package check includes at least one generic scene fixture
- no native editor fallback or game-specific scene flow is introduced

## 10. First Landing Batch

Recommended first batch:

1. `SE-S1-001` scene document header.
2. `SE-S1-002` object identity records.
3. `SE-S1-003` transform records.
4. `SE-S1-008` editor-only sidecar split.
5. `SE-S2-001` duplicate/id validation.

This creates a usable data spine without starting a full editor ecosystem.
