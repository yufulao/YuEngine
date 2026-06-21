# YuEngine Animation Editor Plan

Status: architecture plan
Requested: 2026-06-21
Owner: Architecture
Scope: Animation runtime data, Animation Editor, event/state preview, validation

Related:

- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_WEB_EDITOR_FRONTEND_BOUNDARY.md`
- `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md`
- `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md`
- `docs/YUENGINE_PHASE3_ARCHITECTURE_QUEUE.md`

## 1. Decision

Animation Editor work must start with runtime animation data contracts and a
real engine preview target, then build a narrow timeline/event/state preview
editor around those contracts.

The useful first direction is:

```text
runtime animation clip/state/event data
-> Web editor workspace
-> local animation editor service
-> resource/import/cook validation
-> engine preview host playback viewport and diagnostics
```

The Animation Editor is not a full DCC tool, not a Unity Animator clone, not an
Unreal Sequencer clone, and not a project gameplay FSM editor.
It is also not usable if it only draws tracks and forms. It must show playback
feedback against a model, sprite, UI target, or other runtime preview target.

Animation Editor must share the production loop with UI Editor and Scene
Editor:

- Resource Browser and Import Settings
- stable runtime data formats
- cook and package validation
- runtime preview and diagnostics
- build, run, and package checks

An editor report that only proves a timeline can draw or a state box can be
dragged is incomplete unless the data can pass this loop.

Animation Editor is not usable until it can show:

- a visible runtime preview target
- clip play/pause/scrub/step feedback
- keyframe or sampled-value feedback
- event marker timing feedback
- resource/import diagnostics for missing clip/model/material/texture data
- cooked runtime data that can be loaded by the engine

## 2. Non-Goals

Animation Editor does not own:

- skeletal rig authoring, mesh skinning authoring, IK solvers, retargeting, or
  motion-capture cleanup
- a full timeline cinematic editor
- a full Animator Controller or Blueprint-like visual scripting system
- character movement FSMs, customer/player/stall states, quest triggers, or any
  gameplay behavior graph
- a track-only Web page as core animation preview
- old Unity `Animator`, `Animation`, or `AnimationCurve` API shape
- old project animation helpers as runtime API shape
- asset codec implementation beyond approved Resource/import gates
- native C++ editor app or immediate-mode fallback
- editor-only data inside shipped animation runtime data

External DCC tools should remain responsible for authored source animation
assets. YuEngine should import, validate, preview, and play cooked runtime
animation data.

## 3. Runtime Source Of Truth

The source of truth is runtime animation data, not editor timeline widgets.

The first runtime data families should be:

- animation asset identity and import settings
- clip metadata: duration, sample rate, track count, event count, bounds
- track records: target id, channel kind, sample format, key count
- keyframe/curve records with deterministic interpolation mode
- event marker records with time, event id, payload id, and validation status
- playback command records: play, stop, pause, seek, speed, loop
- optional state-preview records: state id, clip ref, transition condition
  records, blend duration, and explicit invalid-transition status
- preview diagnostics: missing resource, bad time range, unsupported channel,
  event overflow, sample mismatch, and budget failures

Runtime data must not contain:

- editor track colors, foldouts, viewport camera, selection, scrubber UI state,
  or undo history
- gameplay state names or old project character/customer/stall FSM data
- Unity `AnimatorController` or Unreal animation graph API shape
- project config-table dependencies

## 4. Reference Inputs

Reference use is constrained to behavior lessons.

| Reference | Use | Not copied |
| --- | --- | --- |
| old `UtilsForAnimation.cs` | stable animation-name hashing need, play/seek/duration utility expectations | Unity `Animator` API, string-name runtime dependency |
| old `AnimatorTimeUser.cs` and `AnimationTimeUser.cs` | speed/time-scale adjustment requirement | Unity component wrapping and per-component behavior |
| old UI animation snippets | simple UI transition/event expectations | game-specific UI windows and DOTween/Unity behavior |
| Unity Animation/Animator | clip, state, layer, speed, event, transition concepts | Animator Controller asset shape, Mecanim API, Unity state machine semantics |
| Unreal Animation/Sequencer | separation of source asset, runtime playback, notify/event tracks, preview tooling | Sequencer scope, Blueprint/AnimGraph API, editor extensibility |
| YuEngine Resource/Package gates | asset identity, import/cook/package validation | package expansion, old package compatibility |
| YuEngine World/Scene gates | preview attachment to scene objects and resource bindings | gameplay component behavior or scene editor ownership |
| YuEngine editor preview host | visible playback target, frame/status output, resource diagnostics | track-only Web timeline or fake playback |

Each implementation task must record:

```text
Reference used:
Borrowed behavior:
Not copied:
YuEngine-specific acceptance:
```

## 5. Layer Model

### 5.1 Animation Runtime Data

Owns:

- runtime clip identity and version
- deterministic clip metadata
- bounded track descriptors
- bounded keyframe or sampled data descriptors
- interpolation mode values
- event marker records
- playback command/status records
- preview diagnostic records

Does not own:

- editor layout state
- source DCC file editing
- project gameplay state machines
- UI window transition policy
- scene activation or object construction

### 5.2 Animation Runtime Player

Owns:

- clip validation
- playback cursor state
- sample-at-time queries
- loop/clamp behavior
- speed/time-scale application
- event firing window records
- explicit failure statuses
- deterministic counters

First slice should prefer a data-only or headless player. Visual deformation,
skinning, blend trees, IK, root motion, and runtime graph evaluation need later
gates.

### 5.3 Web Animation Editor Workspace

Owns:

- asset/clip browser integration
- timeline track view
- key/event marker editing
- state-preview graph for simple clip transitions
- scrubber and playback controls
- validation result panel
- preview target selection
- editor-only selection, snapping, zoom, track colors, and panel state

The workspace should follow the same Web direction used by the UI Editor:
frontend workflow changes must not require rebuilding YuEngine C++.
However, timeline drawing is not core preview. Playback feedback must come from
the engine preview host.

### 5.4 Local Animation Editor Service

Owns backend support only:

- load/save runtime animation documents
- import settings validation
- schema and version migration
- resource/cook/package validation calls
- preview session control
- exposing a local API for the Web frontend

The service must not encode frontend timeline widgets, graph layout, shortcut
behavior, or state-preview UI in C++.

### 5.5 Runtime Preview

Preview must use the real runtime path:

```text
animation document
-> validate
-> cook/import check
-> headless sample/event pass
-> scene/model/sprite/UI preview target through preview host
-> playback frame/status/event diagnostics
-> runtime diagnostics back to Web editor
```

Scene attachment is optional and must go through Scene/World records. Animation
Editor must not own scene construction.

## 6. Execution Plan

### Stage 0: Boundary Freeze

Goal: make Animation a runtime-data and runtime-preview feature first.

Required:

- document that source DCC authoring remains external
- forbid full Animator/Sequencer clone scope
- forbid gameplay FSM scope
- forbid timeline-only Web pages as usable animation preview
- define initial runtime animation document families
- align with shared Resource Browser, cook/package validator, and runtime
  preview loop

Done when:

- this plan is accepted
- no native animation editor app target is introduced
- no gameplay FSM or old project animation helper is used as runtime API shape

### Stage 1: Animation Runtime Data

Goal: define the runtime-loadable animation data shape.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| AN-S1-001 | Animation document header | schema version, animation id, deterministic document hash |
| AN-S1-002 | Clip metadata | duration, sample rate, track count, event count, loop mode |
| AN-S1-003 | Track descriptors | target id, channel kind, sample format, key count |
| AN-S1-004 | Keyframe records | bounded key buffers and interpolation mode validation |
| AN-S1-005 | Event marker records | event id, time range, payload id, duplicate/overflow status |
| AN-S1-006 | Import settings record | source asset logical key, clip range, sample rate, compression policy |
| AN-S1-007 | Editor-only sidecar | timeline zoom, selection, colors, graph layout kept separate |

### Stage 2: Animation Runtime Player And Validation

Goal: prove runtime behavior without a full graph engine.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| AN-S2-001 | Clip validator | rejects invalid duration, sample rate, track/key/event counts |
| AN-S2-002 | Sample-at-time path | deterministic sample output for bounded scalar/vector channels |
| AN-S2-003 | Playback cursor | play, stop, pause, seek, loop, clamp, speed status tests |
| AN-S2-004 | Time-scale handling | speed multiplier behavior follows explicit runtime records |
| AN-S2-005 | Event window query | events emitted for deterministic time intervals without duplicates |
| AN-S2-006 | No-hidden-allocation proof | sampled preview path does not allocate/grow dynamically |
| AN-S2-007 | Cook/package validation | source/import/runtime data validated against Resource/Package state |

### Stage 3: Simple State And Event Preview

Goal: preview clip transitions and event timing without owning gameplay FSMs.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| AN-S3-001 | State-preview records | state id, clip ref, speed, loop, entry status |
| AN-S3-002 | Transition records | from, to, condition id, blend duration, explicit failure status |
| AN-S3-003 | Blend preview | deterministic two-clip blend sample for bounded channels |
| AN-S3-004 | Event preview | event markers shown and emitted through runtime event window query |
| AN-S3-005 | Invalid transition diagnostics | missing clip/state/condition rejected without mutation |

This is a preview/data feature. It must not become a gameplay behavior tree,
character controller, or quest/action graph.

### Stage 4: Web Animation Editor Workspace

Goal: build the authoring surface around runtime animation data and engine
playback preview.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| AN-EW-001 | Clip browser | uses shared Resource Browser and Import Settings |
| AN-EW-002 | Timeline view | draws runtime tracks/keyframes/events from document data |
| AN-EW-003 | Inspector | edits runtime fields and import settings, not runtime-invisible UI state |
| AN-EW-004 | Event editor | adds, moves, deletes event markers with validator feedback |
| AN-EW-005 | State preview graph | simple state/transition preview only, no gameplay FSM |
| AN-EW-006 | Engine playback viewport | visible target/frame/status comes from preview host |
| AN-EW-007 | Scrub/play controls | uses runtime preview protocol for sample/event result |
| AN-EW-008 | Keyframe feedback | key/sample edits produce visible runtime target changes |
| AN-EW-009 | Validation panel | shows import, schema, event, sampling, and package diagnostics |
| AN-EW-010 | Frontend test route | editor UI/data tests pass without CMake rebuild |

### Stage 5: Scene Preview Integration

Goal: allow Animation preview against Scene data without coupling ownership.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| AN-PV-001 | Preview target binding | selected scene object/resource binding is read through Scene/World data |
| AN-PV-002 | Visible playback target | model, sprite, UI, or equivalent target displays clip output |
| AN-PV-003 | Headless sample preview | clip samples and events returned without visual target dependency |
| AN-PV-004 | Render preview hook | preview frame comes through runtime render path |
| AN-PV-005 | Step/scrub feedback | play/pause/scrub/step produces deterministic frame/event output |
| AN-PV-006 | Diagnostics snapshot | sample count, event count, active state, missing resources, failed refs |
| AN-PV-007 | Build/run/package check | cooked animation fixture participates in package validation |

## 7. Hard Blocks

These are blocking violations:

- copying Unity Animator, Animation, AnimationController, or Mecanim API shape
- copying Unreal Sequencer, Blueprint, or AnimGraph API shape
- making game-specific character/customer/stall/player FSMs part of Animation
  Editor scope
- using old project animation helpers as runtime API shape
- accepting a Web timeline, 2D canvas, HTML forms, or static screenshots as
  animation preview
- calling Animation Editor usable before visible playback, scrub/step, keyframe
  feedback, and event timing feedback exist on a runtime preview target
- adding a native animation editor app or immediate-mode fallback
- making Web editor logic part of shipped runtime
- hiding runtime state in editor-only timeline or graph layout data
- validating progress only by drawing a timeline or node graph
- expanding Resource/Package/File/Scene behavior without approved gates
- requiring CMake rebuild for ordinary timeline, graph, template, or inspector
  frontend changes

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
ctest --preset windows-fast-gate -R "(Animation|Resource|Package|WorldScene|RenderScene)" --output-on-failure
```

Frontend-only changes should add an Animation Editor Web command analogous to
the UI Web route, and that command must not require a C++ rebuild.

## 9. Completion Definition

Animation Editor first round is complete only when:

- runtime animation document schema is stable
- import settings and Resource Browser are shared
- clip validator and runtime player first slice pass focused tests
- event marker and playback cursor behavior are deterministic
- visible preview target and playback frame/status path come from engine preview
  host
- keyframe, scrub, step, and event marker changes produce runtime feedback
- state-preview records remain separate from gameplay FSMs
- editor-only timeline/graph state is separated from runtime data
- runtime preview returns sample/event/diagnostic status through engine paths
- cook/package validation covers animation dependencies
- no native editor fallback, DCC authoring scope, or game-specific animation
  behavior is introduced

## 10. First Landing Batch

Recommended first batch:

1. `AN-S1-001` animation document header.
2. `AN-S1-002` clip metadata.
3. `AN-S1-003` track descriptors.
4. `AN-S1-005` event marker records.
5. `AN-S1-007` editor-only sidecar split.

This creates a runtime-data spine before any timeline UI or state-preview work.
