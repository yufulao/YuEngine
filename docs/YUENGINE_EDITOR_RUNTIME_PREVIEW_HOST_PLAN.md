# YuEngine Editor Runtime Preview Host Plan

Status: corrective architecture gate
Requested: 2026-06-21
Owner: Architecture
Scope: shared editor preview host, viewport, resource preview, editor acceptance floor

Related:

- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
- `docs/gates/EDITOR_GATE_002_SCENE_EDITOR_DEPENDENCY_GATE.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_ANIMATION_EDITOR_PLAN.md`
- `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
- `docs/YUENGINE_EDITOR_DEPENDENCY_CHAIN_NO_BUILD_LIST.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV6_EDITOR_VIEWPORT_INTERACTION.md`

## 1. Correction

The editor direction is not "build Web forms first and improve them later".
Web is no longer a YuEngine editor direction.

For UI, Scene, and Animation, the editor is only credible when it is a visual
entry point into YuEngine runtime capability:

```text
runtime data
-> resource/import/cook validation
-> engine preview host
-> engine-rendered viewport or frame output
-> native/engine editor panels and commands
```

The editor surface is native/engine-hosted tooling: hierarchy, inspector,
resource browser, timeline, validation panels, command routing, and workflow.
Web shells are deprecated historical scaffolding and are not editor direction.

Forbidden HTML/CSS, 2D canvas sketches, browser-only output, static screenshots, and
administrative-form style pages are not acceptable core preview capability for a
game engine editor.

## 2. Minimum Usable Editor Floor

The target is not a full Unity or Unreal ecosystem, but the interaction floor
must be understood as Unity-like usability:

- viewport, not only forms
- selectable objects/widgets/tracks
- camera or viewport controls
- transform gizmo where spatial data is edited
- real resource loading path for models, textures, materials, sprites, fonts,
  clips, and audio where relevant
- real engine runtime render or headless preview path
- runtime data saved in engine formats
- cook/package validation before the data is called shippable
- diagnostics for missing resources, broken bindings, unsupported channels,
  bad formats, capacity failures, and preview failures

Anything below this is a data/configuration page, not a usable engine editor.

## 2.1 L0/L1 Boundary Correction

Completing the historical L0/L1 execution plan does not satisfy the editor
visual requirement.

L0/L1 proves foundation only:

- file, resource, package, and value-record paths exist
- RHI can clear, draw primitive fixtures, sample a texture, present, and capture
- RenderCore can execute bounded fixture passes
- RenderScene can produce RenderCore packet values
- sample programs can prove isolated vertical smoke paths

These are prerequisites, not editor preview acceptance. A D3D11 sample
screenshot or a capture-byte fixture must not be reported as Scene Editor,
Animation Editor, UI Editor, or Preview Host completion.

The first credible visual proof must run through a YuEngine preview-host path
that combines the required runtime layers into one frame sequence:

```text
preview session
-> resource/model/texture/material resolution
-> scene/object transform records
-> material texture-slot binding
-> shader/pipeline selection
-> RenderScene multi-entity submission
-> RenderCore/RHI multi-draw frame
-> camera-controlled frame capture sequence
```

The minimum scene-proof sample is intentionally concrete:

- one cube, one cylinder, and one cone
- deterministic pseudo-random placement from a fixed seed
- per-object rotation over time
- one material record applied to all three objects
- at least three distinct texture inputs bound by that material record
- an orbit camera that captures a full turn as a bounded screenshot/frame set

If any layer in that chain is missing, the correct status is a blocked or
partial preview-host implementation, not "L0/L1 complete".

## 2.2 Runtime Asset v0 Boundary

The first preview-host MVP is after RuntimeAsset v0, not a replacement for it.
RuntimeAsset v0 means the current `YuRuntimeAsset` and `RuntimeAssetData_*`
closed-loop smoke can load generated runtime files through File/VFS, Resource,
Asset, RenderScene, RenderCore, and RHI. It still carries production gaps:
complete typed source/runtime file contracts, decoded texture payload consumption
by RenderScene materials, shader bytecode/program ownership, disk animation
sampling, and production scene loader output APIs remain under the runtime asset
contract work.

The Preview Host MVP consumes that v0 route:

```text
RuntimeAsset v0 loaded files and statuses
-> Resource/Asset handles and dependency diagnostics
-> RenderScene records
-> RenderCore/RHI frame or headless output
-> preview session frame/status/diagnostic records
```

It must not choose source asset formats, import metadata, cook/package ownership,
or production loader policy. If the canonical cube/cylinder/cone proof reaches a
RuntimeAsset production gap, the Preview Host returns an explicit blocker or
partial status instead of substituting fixture structs, editor mock data,
screenshots, or report/oracle output.

The MVP boundary for #YuPart task #38 is therefore:

| Interface | Owned here | Owned elsewhere |
| --- | --- | --- |
| Session lifecycle | start/update/stop, stale-session failure, shutdown release | native editor app lifecycle |
| Frame/status/diagnostics | caller-owned frame/headless descriptors, bounded diagnostics, explicit capacity failure | report/oracle schema or screenshot acceptance |
| Camera/orbit | preview camera records that affect runtime output identity/status | full editor viewport UX and shortcuts |
| Runtime resource preview bridge | RuntimeAsset/Resource/Asset refs and missing/stale/type mismatch diagnostics | Resource Browser/import settings UX (#39) |
| Selection/transform feedback | bounded hit, selection, and transform feedback records from preview output | Scene editor gizmo workflow and command stack (#40) |
| Canonical scene proof | loaded runtime files produce cube/cylinder/cone frame sequence, or exact #36 blocker is returned | source data format and production loader gap closure (#36) |

## 3. Shared Architecture

### 3.1 Native/Engine Editor Surface

Owns:

- navigation, tabs, panels, hierarchy, inspector, timeline, and property editing
- resource browser UI and import settings UI
- selection state, layout state, keyboard shortcuts, undo/redo, and editor-only
  commands
- validation result display
- calling the local service and preview host

Does not own:

- game/runtime rendering
- scene camera rendering
- material/texture/shader evaluation
- animation sampling authority
- UI runtime layout/render authority
- resource loading/cook/package truth

### 3.2 Local Editor Service

Owns:

- file load/save
- schema migration
- validation command routing
- cook/package command routing
- preview session lifecycle
- local editor-host bridge to the engine preview host

Does not own:

- frontend panels or workflow
- runtime behavior
- renderer shortcuts that bypass engine systems

### 3.3 Engine Preview Host

Owns:

- loading runtime data into an isolated preview world/session
- resource resolution through engine Resource/Package/File paths
- model, texture, material, sprite, font, animation, UI, render, and audio
  preview hooks as each subsystem becomes available
- viewport camera state and frame output
- transform, selection, hit, and diagnostics feedback
- headless preview outputs when visual output is not required

The host must be an engine capability. It may expose frames, snapshots, IDs,
diagnostics, and hit/selection results to the editor surface, but browser or
deprecated Web output must not fake these as authoritative runtime output.

## 4. Required Landing Order

The editor plan order is:

```text
L0/L1 runtime foundation only
-> RuntimeAssetData v0 disk-backed closed loop
-> Editor visual capability correction
-> Engine Preview Host MVP gate
-> Resource Browser / Import Settings scope
-> RenderScene / Material / Texture / Shader preview path
-> Scene runtime format and viewport
-> Scene Editor
-> Animation runtime clip/track/event preview
-> Animation Editor
-> UI runtime render preview
-> UI Editor
-> Cook / Package / Run smoke
```

Resource Browser/import work may be planned in parallel, but it is not allowed
to stand in for preview-host output. Likewise, Scene/Animation/UI editor plans
may keep data-schema preparation work, but they must not claim usable editor
progress until their runtime data can pass validation and the shared preview host
can return authoritative runtime output or an exact blocker.

Removed Web workspace scaffolding is residual cleanup context only. It is not
usable-editor progress; engine UI runtime preview must be visible and validated
through the host.

Scene work can keep scene data/schema scaffolding, but it is not usable-editor
progress until the viewport can load and inspect models/materials/textures with
camera and transform controls.

Animation work can keep clip/event data scaffolding, but it is not
usable-editor progress until playback, scrub, event timing, and keyframe
feedback are visible against a model, sprite, or other runtime preview target.

## 5. Deprecated Web Slices

Removed historical browser-editor folders are deprecated scaffolding. They must
not be connected forward as an editor compatibility path and must not be
reported as usable editor completion.

In particular:

- a deprecated UI Web page without engine UI runtime rendering is not usable UI
  Editor progress
- a deprecated Scene Web canvas without engine-rendered model/material/texture
  viewport is not usable Scene Editor progress
- a deprecated Animation Web timeline without visible runtime playback target
  is not usable Animation Editor progress

## 6. First Preview Host Batch

Task #65 implements the first bounded `YuPreviewHost` engine target documented
in `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`. That slice covers value/session
records, RuntimeAsset loaded graph/status/resource-ref validation, bounded
diagnostics, headless/capture frame output, and selection/hit/transform feedback
records. It remains below editor panels and does not define source asset
formats, Resource Browser import settings UX, or editor mock scenes.

Recommended MVP gate batch after RuntimeAsset v0:

| ID | Work item | Acceptance |
| --- | --- | --- |
| EPV-001 | Preview host process/session contract | editor host can start/stop isolated engine preview sessions |
| EPV-002 | Viewport frame protocol | host can return a frame/status/diagnostics payload to editor tooling |
| EPV-003 | Preview camera contract | orbit/pan/zoom or equivalent camera state affects engine viewport output |
| EPV-004 | RuntimeAsset v0 resource preview bridge | model/texture/material refs resolve through RuntimeAsset/Resource/Asset path or return explicit bounded diagnostics |
| EPV-005 | Transform and selection feedback | preview returns bounded hit, selection, and transform feedback records; editor overlays remain editor-only |
| EPV-006 | Canonical scene visual proof or blocker | cube/cylinder/cone scene with three-texture material, object rotation, orbit camera, and bounded captured frame set comes from loaded runtime files, or reports the exact #36 production-gap blocker |

Later editor-facing batches:

| ID | Work item | Acceptance |
| --- | --- | --- |
| EPV-LATER-001 | Resource Browser / Import Settings integration | shared browser and typed import settings consume preview diagnostics without owning runtime truth |
| EPV-LATER-002 | UI runtime preview hook | UI layout renders through engine UI runtime, not HTML/CSS |
| EPV-LATER-003 | Animation playback preview hook | play/pause/scrub/step returns sampled state and visible target feedback |
| EPV-LATER-004 | Scene viewport command bridge | transform gizmo and viewport commands update runtime data through approved scene/world gates |
| EPV-LATER-005 | Cook/package/run smoke bridge | previewed data can be validated as cook/package/run input after owning asset/package gates close |

## 7. Hard Blocks

These are blocking violations:

- claiming L0/L1 completion, RHI fixture capture, RenderCore fixture pass, or
  isolated sample screenshots satisfy editor preview capability
- claiming RuntimeAsset v0 smoke closes the full production asset contract or
  lets Preview Host define source asset formats
- accepting deprecated Web shell, form UI, 2D canvas sketches, or static screenshots as
  core editor preview
- treating CSS/HTML visual output as the game editor's authoritative preview
- calling Scene Editor usable without model/texture/material loading entry,
  camera controls, transform gizmo, and engine-rendered viewport
- calling Animation Editor usable without visible playback, scrub, keyframe or
  event feedback on a runtime preview target
- calling UI Editor usable without engine UI runtime render preview
- optimizing a deprecated Web-form surface instead of adding engine preview capability
- using fake preview data that cannot be cooked, packaged, and loaded by runtime
- bypassing loaded runtime files with in-memory fixture structs, editor-only mock
  data, report/oracle output, or screenshots for the canonical scene proof
- expanding gameplay/product logic to mask missing engine preview foundations

## 8. Completion Definition

The #YuPart task #38 MVP gate is complete when:

- this plan and `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md` define
  Preview Host as an after-RuntimeAsset-v0 consumer;
- MVP interfaces include session, frame/status/diagnostics, camera/orbit,
  runtime resource preview bridge, and selection/transform feedback;
- the canonical cube/cylinder/cone proof requires loaded runtime files and
  reports exact #36 production-gap blockers instead of bypassing them;
- Resource Browser/import settings ownership remains with #39;
- Scene/Animation/UI dependency ordering and no-build-yet items remain with #40;
- documentation-only verification passes with `git diff --check`.

The shared editor foundation is complete only when:

- editor tooling can control an engine preview session
- preview frames or headless runtime outputs come from YuEngine systems
- resources are resolved through engine Resource/Package/File paths
- resource preview requests come from the Resource Browser/import/cook scope
  after validation, not from editor-only fake assets
- camera/viewport controls affect engine output
- the canonical cube/cylinder/cone scene proof can be captured through the
  preview host, or the exact missing runtime layer is reported as a blocker
- Scene, Animation, and UI editor data can each be validated against runtime
  data formats
- cook/package/run smoke can consume the same data
- diagnostics identify missing resources, bad bindings, unsupported data, and
  capacity failures
