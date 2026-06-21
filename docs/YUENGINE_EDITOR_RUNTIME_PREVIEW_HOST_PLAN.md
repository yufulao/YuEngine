# YuEngine Editor Runtime Preview Host Plan

Status: corrective architecture gate
Requested: 2026-06-21
Owner: Architecture
Scope: shared editor preview host, viewport, resource preview, editor acceptance floor

Related:

- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_ANIMATION_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_WEB_EDITOR_FRONTEND_BOUNDARY.md`

## 1. Correction

The editor direction is not "build Web forms first and improve them later".

For UI, Scene, and Animation, the editor is only credible when it is a visual
entry point into YuEngine runtime capability:

```text
runtime data
-> resource/import/cook validation
-> engine preview host
-> engine-rendered viewport or frame output
-> Web workspace panels and commands
```

Web is the workspace shell: hierarchy, inspector, resource browser, timeline,
validation panels, command routing, and workflow. Web is not the authoritative
preview renderer.

HTML/CSS, 2D canvas sketches, static screenshots, and administrative-form style
pages are not acceptable core preview capability for a game engine editor.

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

## 3. Shared Architecture

### 3.1 Web Workspace

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
- IPC/WebSocket/HTTP bridge between Web workspace and engine preview host

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
diagnostics, and hit/selection results to Web, but Web must not fake these as
authoritative runtime output.

## 4. Required Landing Order

The editor plan order is:

```text
L0/L1 runtime foundation only
-> Editor visual capability correction
Engine Preview Host
-> Resource Browser / Import Settings
-> RenderScene / Material / Texture / Shader preview path
-> Scene runtime format and viewport
-> Scene Editor
-> Animation runtime clip/track/event preview
-> Animation Editor
-> UI runtime render preview
-> UI Editor
-> Cook / Package / Run smoke
```

UI work can keep Web workspace scaffolding, but it is not usable-editor progress
until engine UI runtime preview is visible and validated through the host.

Scene work can keep scene data/schema scaffolding, but it is not usable-editor
progress until the viewport can load and inspect models/materials/textures with
camera and transform controls.

Animation work can keep clip/event data scaffolding, but it is not
usable-editor progress until playback, scrub, event timing, and keyframe
feedback are visible against a model, sprite, or other runtime preview target.

## 5. Current Web Slices

Current folders such as:

- `Tools/UiWebEditorWeb`
- `Tools/SceneWebEditorWeb`
- `Tools/AnimationWebEditorWeb`

are allowed only as Web workspace scaffolding unless and until they connect to
the engine preview host. They must not be reported as usable editor completion
by themselves.

In particular:

- a UI Web page without engine UI runtime rendering is not usable UI Editor
  progress
- a Scene Web canvas without engine-rendered model/material/texture viewport is
  not usable Scene Editor progress
- an Animation Web timeline without visible runtime playback target is not
  usable Animation Editor progress

## 6. First Preview Host Batch

Recommended first batch:

| ID | Work item | Acceptance |
| --- | --- | --- |
| EPV-001 | Preview host process/session contract | Web can start/stop isolated engine preview sessions |
| EPV-002 | Viewport frame protocol | host can return a frame/status/diagnostics payload to Web |
| EPV-003 | Preview camera contract | orbit/pan/zoom or equivalent camera state affects engine viewport output |
| EPV-004 | Resource preview bridge | model/texture/material/sprite/clip refs resolve through engine resource path or return explicit diagnostics |
| EPV-005 | Transform and selection feedback | viewport supports select and translate/rotate/scale feedback for scene objects |
| EPV-006 | UI runtime preview hook | UI layout renders through engine UI runtime, not HTML/CSS |
| EPV-007 | Animation playback preview hook | play/pause/scrub/step returns sampled state and visible target feedback |
| EPV-008 | Cook/package smoke bridge | previewed data can be validated as cook/package/run input |
| EPV-009 | Canonical scene visual proof | cube/cylinder/cone scene with three-texture material, object rotation, orbit camera, and bounded captured frame set comes from YuEngine preview host |

## 7. Hard Blocks

These are blocking violations:

- claiming L0/L1 completion, RHI fixture capture, RenderCore fixture pass, or
  isolated sample screenshots satisfy editor preview capability
- accepting Web shell, form UI, 2D canvas sketches, or static screenshots as
  core editor preview
- treating CSS/HTML visual output as the game editor's authoritative preview
- calling Scene Editor usable without model/texture/material loading entry,
  camera controls, transform gizmo, and engine-rendered viewport
- calling Animation Editor usable without visible playback, scrub, keyframe or
  event feedback on a runtime preview target
- calling UI Editor usable without engine UI runtime render preview
- optimizing a bad Web-form surface instead of adding engine preview capability
- using fake preview data that cannot be cooked, packaged, and loaded by runtime
- expanding gameplay/product logic to mask missing engine preview foundations

## 8. Completion Definition

The shared editor foundation is complete only when:

- Web can control an engine preview session
- preview frames or headless runtime outputs come from YuEngine systems
- resources are resolved through engine Resource/Package/File paths
- camera/viewport controls affect engine output
- the canonical cube/cylinder/cone scene proof can be captured through the
  preview host, or the exact missing runtime layer is reported as a blocker
- Scene, Animation, and UI editor data can each be validated against runtime
  data formats
- cook/package/run smoke can consume the same data
- diagnostics identify missing resources, bad bindings, unsupported data, and
  capacity failures
