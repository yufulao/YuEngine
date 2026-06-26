# YuEngine Runtime Visual Foundation Plan

Status: Draft correction after task #40

Owner: Architecture

Scope: pure engine runtime capability ladder for camera, scene placement,
geometry/model, material/texture/shader binding, animation interpolation, render
submission, and runtime capture
Related runtime data contract:
`docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md` and
`docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`

This document exists because the runtime visual requirement must be built from
basic module capabilities, not from a single late "preview" milestone. A camera
that cannot produce view/capture proof, a scene system that cannot place
objects, a material system that cannot bind texture slots, or an animation
runtime that cannot interpolate keyframes is not a closed engine foundation.

This is not editor scope, input scope, UI scope, Web scope, or gameplay scope.

## 1. Principle

Work must land from shallow to deep:

1. Prove each module's smallest useful runtime behavior.
2. Make every proof deterministic and command-line runnable.
3. Combine modules only after their individual proofs exist.
4. Report the exact missing module layer instead of hiding it behind a broad
   "L0/L1 passed" statement.

The final cube/cylinder/cone orbit capture is the integration result. It is not
the first step.

## 2. Capability Ladder

| Stage | Capability | Owner layer | Minimum proof | Cannot be replaced by |
| --- | --- | --- | --- | --- |
| RVF-001 | RHI/RenderCore capture primitive | L0 | clear/present/capture and indexed draw capture on supported D3D11 hardware, or explicit `BlockedByEnv` | screenshots taken manually, standalone D3D sample |
| RVF-002 | Camera value math | L1 RenderScene/Camera | camera record produces deterministic view/projection values from position, target, FOV, aspect, near/far | editor camera, hard-coded sample matrix without owned camera record |
| RVF-003 | Camera capture route | L0/L1 boundary | runtime command records frame index, camera pose, capture target, and bounded output path/status | UI viewport, manual screenshot, report-only proof |
| RVF-004 | Geometry/model basis | L0 Resource/RenderCore + L1 Asset | cube, cylinder, and cone geometry records or model assets resolve to vertex/index/draw records with explicit bounds | one triangle, one cube only, product scene mesh |
| RVF-005 | Scene placement | L1 World/RenderScene | fixed-seed placement creates at least three object transform records and three render entities with distinct transforms | a single mesh repeated only inside a backend fixture |
| RVF-006 | Material texture slots | L0 RenderCore/RHI + L1 Asset/Material | one material record owns at least three texture inputs and binds them by slot to a draw submission | one `texture=` sample field, per-object ad hoc texture binding |
| RVF-007 | Shader/pipeline binding | L0 RenderCore/RHI | pipeline and shader constants/texture slots are bound through YuEngine value contracts and validated for missing/invalid handles | hard-coded native backend state hidden from engine records |
| RVF-008 | Animation interpolation | L1 Animation | bounded keyframe tracks interpolate scalar/vector/rotation or transform channels at deterministic times | per-frame sample-only math, editor timeline, gameplay behavior |
| RVF-009 | Runtime transform application | L1 Animation + World/Transform | sampled animation output applies to object transform records before RenderScene consumes them | direct mutation inside RenderCore/RHI |
| RVF-010 | Multi-entity render submission | L1 RenderScene + L0 RenderCore | three entities submit as bounded draw records in one runtime frame with shared material and distinct transforms | RenderScene packet-only value tests without draw submission |
| RVF-011 | Camera tween capture sequence | Runtime sample | explicit perspective camera tween keyframes emit a bounded frame/capture set with per-frame status | single still image, editor preview, manual capture |
| RVF-012 | Missing-layer diagnostics | Runtime sample | failure names one of camera, geometry/model, material slots, shader/pipeline, scene placement, animation interpolation, RenderScene submission, RenderCore/RHI draw/capture, or resource resolution | generic "unsupported", "not implemented", or environment skip for semantic gaps |

## 3. Required Module Floors

### 3.1 Camera

Minimum floor:

- camera record with position, target or orientation, FOV, aspect, near/far;
- deterministic view/projection calculation;
- active camera selection for a runtime frame;
- capture route that records camera pose and frame index.

The first camera test may be value-only. Closure requires at least one runtime
capture command using the owned camera record.

### 3.2 Scene Placement

Minimum floor:

- object identity and transform records already owned by L1 World/Object;
- fixed-seed placement function for sample objects;
- scene record binds object transform, geometry/model, and material handle;
- RenderScene consumes the list without native backend ownership.

The scene floor is not complete if the engine can only draw one hard-coded mesh.

### 3.3 Geometry / Model

Minimum floor:

- cube, cylinder, and cone are represented as bounded geometry records or
  checked-in synthetic model assets;
- vertex/index counts, topology, and draw ranges are explicit;
- Resource/Asset resolution reports missing or invalid geometry by status.

Procedural primitives are acceptable for this floor if they are YuEngine-owned
runtime data and go through the same draw path as imported models.

### 3.4 Material / Texture / Shader

Minimum floor:

- material record has stable ID/generation and at least three texture slots;
- each texture slot resolves to a sampled texture binding and sampler value;
- shader/pipeline binding consumes the material slots through RenderCore/RHI
  value contracts;
- missing slot, invalid texture, invalid sampler, or invalid pipeline returns an
  explicit status.

A single texture sample or a backend-private texture bind does not close this
floor.

### 3.5 Animation

Minimum floor:

- animation clip/track/keyframe records are bounded and runtime-owned;
- interpolation supports deterministic scalar/vector/rotation or transform
  channels;
- runtime time from `FrameContext` samples an animation track;
- sampled output can be applied to an object transform record;
- missing clip, missing track, unsupported interpolation, out-of-range time, and
  capacity overflow return explicit statuses.

The cube/cylinder/cone rotation may start as a simple transform update only if
`L1-ANIM-*` remains a named blocker. L1 visual closure is stronger when the
rotation is driven by the animation sampler.

### 3.6 Capture

Minimum floor:

- capture output is emitted by a checked-in runtime command;
- frame count, output naming, and capture memory/file bounds are deterministic;
- generated files are not committed;
- capture absence caused by D3D11/display limitations is `BlockedByEnv`;
- capture absence caused by missing engine semantics is `Fail` or a named
  missing-layer blocker.

## 4. Integration Sequence

Do not jump directly to the final scene. Land the proofs in this order:

1. Static camera + one cube capture.
2. Camera moved to a second pose, same cube, proving camera values affect
   capture.
3. Three static primitives placed by fixed seed.
4. Shared material with three texture slots applied to all primitives.
5. Per-object transform update from runtime time.
6. Per-object transform update from animation interpolation.
7. Orbit camera full-turn bounded capture set.
8. Failure-path command that reports the exact first missing layer.

Each step must be independently runnable or explicitly recorded as a blocker.

## 5. Backlog IDs

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-CAMERA-001 | Define runtime camera record | L1-KERN-003 | position/orientation or target, FOV, aspect, near/far produce deterministic view/projection values |
| L1-CAMERA-002 | Active camera frame binding | L1-CAMERA-001, L1-RSCENE-004 | RenderScene frame references one active camera record and rejects missing camera |
| L1-CAMERA-003 | Camera capture metadata | L1-CAMERA-002, L0-RHI-002 | runtime capture records frame index, camera pose, capture target/status, and bounded output metadata |
| L1-GEOM-001 | Runtime primitive geometry records | L0-REN-003, L1-ASSET-001 | cube/cylinder/cone geometry records expose bounded vertex/index/draw ranges |
| L1-MAT-001 | Runtime material texture-slot record | L0-REN-004, L1-ASSET-005 | one material binds at least three texture inputs by slot with explicit missing/invalid statuses |
| L1-ANIM-001 | Animation clip/track/keyframe records | L1-KERN-003 | bounded runtime records exist without editor timeline or gameplay dependency |
| L1-ANIM-002 | Interpolation sampler | L1-ANIM-001 | deterministic scalar/vector/rotation or transform interpolation tests pass at fixed times |
| L1-ANIM-003 | Runtime time sampling | L1-ANIM-002 | `FrameContext` time samples tracks without hidden global time |
| L1-ANIM-004 | Apply sampled transform | L1-ANIM-003, L1-OBJ-003 | sampled output updates object transform records before RenderScene consumes them |
| L1-ANIM-005 | Animation failure states | L1-ANIM-001 | missing clip/track, unsupported interpolation, out-of-range time, and capacity overflow return explicit statuses |
| L1-VIS-001 | Static one-cube capture | L1-CAMERA-003, L1-GEOM-001, L1-MAT-001 | checked-in runtime command captures one cube through YuEngine modules |
| L1-VIS-002 | Three-primitive placed scene | L1-VIS-001, L1-RSCENE-006 | cube/cylinder/cone submit as three entities with fixed-seed transforms |
| L1-VIS-003 | Shared three-texture material scene | L1-VIS-002, L1-MAT-001 | all primitives use one material with three distinct texture inputs |
| L1-VIS-004 | Animated transform scene | L1-VIS-002, L1-ANIM-004 | object rotations are driven by runtime animation or the command reports `L1-ANIM-*` blocker |
| L1-VIS-005 | Camera tween capture sequence | L1-VIS-003, L1-VIS-004 | explicit perspective camera tween keyframes emit bounded frame/capture set |
| L1-VIS-006 | Missing-layer diagnostic route | L1-VIS-005 | failure names the exact missing layer from section 2 |

## 6. Closure Rule

L0/L1 may not be called complete by saying "the final sample is planned." The
module floors above must either pass in order or produce explicit blocker rows.
The final cube/cylinder/cone camera tween capture is valid only when it is the
composition of these module floors.

If the final scene claims runtime asset/data loading, it must also satisfy the
runtime asset/data closed loop gate: generator-written disk files read through
File/VFS/Resource, validated/cooked/loaded into runtime records/resources, then
rendered and captured through RenderScene, RenderCore, and RHI. RVF helper
image artifacts, CPU semantic PPM output, and GDI/software viewer output remain
auxiliary evidence only.
