# EDITOR-GATE-001: Runtime Preview Host

Status: Proposed architecture/reference gate
Requested decision: `REFERENCE_GATE_REVIEW`
Current decision: `REVIEW_ONLY_NOT_IMPLEMENTATION_APPROVED`
Owner: Architecture
Task: #70
Source baseline: `5dabfae`
Related plans:

- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_ANIMATION_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_WEB_EDITOR_FRONTEND_BOUNDARY.md`

## Purpose

Define the shared editor runtime preview host boundary before UI, Scene, and
Animation editor work claims usable editor progress.

This gate is architecture and reference guidance only. It does not authorize
implementation, does not create a native editor application, and does not allow
Web, HTML/CSS, 2D canvas sketches, or static screenshots to become authoritative
runtime preview.

The preview host is the bridge between Web editor workspaces and YuEngine
runtime capability:

```text
Web workspace commands
-> local editor service
-> isolated engine preview host session
-> runtime validation / resource resolution / frame or headless output
-> bounded status, diagnostics, hit, selection, and frame records back to Web
```

## Layer

L7 editor tooling over approved runtime modules.

The preview host may call approved engine runtime surfaces to build an isolated
preview session. It must not become a lower-layer dependency and must not make
shipped runtime modules depend on Web editor code.

## Reference Boundary

Reference engines are used only for responsibility separation:

- Unity and Unreal both treat editor workspaces, runtime preview viewports,
  resource validation, and play/simulate sessions as separate responsibilities.
- YuEngine should borrow that separation: Web owns workspace panels and editor
  commands; the preview host owns authoritative runtime preview output.
- YuEngine must not copy Unity Scene/GameObject/Animator/UGUI APIs, Unreal
  World/Actor/Component/Slate APIs, plugin ecosystems, asset database shape, or
  editor extensibility model in this gate.

Reference behavior to preserve:

- viewport output comes from the engine runtime path;
- selection, hit, transform, playback, UI layout, and resource diagnostics are
  returned as explicit records;
- editor-only camera, selection, overlays, foldouts, timeline zoom, and undo
  state are separate from runtime data;
- cook/package/resource validation happens before data is considered shippable.

## Owns

This gate owns the architecture contract for a future shared preview host:

- preview session identity and lifecycle records;
- explicit start, update, frame/headless-output, diagnostic, and stop statuses;
- isolated preview world/session state that is not the game runtime state;
- preview document kinds for UI, Scene, Animation, and resource-focused preview;
- viewport camera state records;
- frame output descriptors and bounded caller-owned frame buffers;
- headless output records when visual output is not required;
- hit-test, selection, transform, playback, and UI layout feedback records;
- resource reference diagnostics for missing, stale, type-mismatched,
  unsupported, or not-yet-cooked resources;
- preview diagnostics, counters, and snapshots with fixed capacities;
- IPC/WebSocket/HTTP protocol shape between local editor service and Web
  workspace;
- no-mutation failure semantics for invalid requests and insufficient output
  capacity;
- explicit session shutdown behavior and stale-session rejection.

The host may expose runtime-derived data to Web. Web may draw overlays such as
selection boxes, transform handles, grid, labels, and timeline markers, but
those overlays are editor-only and never authoritative runtime output.

## Does Not Own

This gate does not own:

- Web editor panel layout, hierarchy, inspector, resource browser, timeline,
  shortcuts, drag/drop, undo/redo, or workflow behavior;
- a native editor app, immediate-mode editor fallback, or C++ Web editor shell;
- HTML/CSS/DOM/canvas as authoritative game/editor preview;
- UI runtime implementation, Scene runtime implementation, Animation runtime
  implementation, RenderScene, AudioScene, Resource, Package, or File expansion;
- material graph editing, shader tooling, animation graph authoring,
  gameplay FSMs, physics authoring, scripting IDE behavior, save slots, scene
  loading policy, or Game Adapter behavior;
- old TouhouNewWorld editor, scene, UI, animation, package, save, report, or
  runtime compatibility behavior;
- report/oracle/tool output as runtime behavior;
- unbounded background preview processes or hidden global editor services;
- remote/cloud preview execution.

## Required Architecture Rules

1. Web workspace is a client, not preview authority.
2. Local editor service routes load/save/validate/cook/preview commands. It must
   not own frontend panels or runtime behavior.
3. Preview host sessions are isolated from the normal game runtime and must be
   explicitly started and stopped.
4. Runtime data is the source of truth. Editor-only state must be sidecar data.
5. Preview resource resolution must use approved Resource/Package/File paths or
   return explicit diagnostics.
6. Preview cannot bypass approved restore/preflight/proof gates when Scene or
   World data is involved.
7. UI preview must use YuEngine UI runtime output, not browser layout.
8. Animation preview must use runtime clip/player/sample/event output, not only
   timeline drawing.
9. Scene preview must use engine-rendered or engine-headless scene outputs, not
   a standalone Web canvas mock.
10. Failures must be explicit and no-mutation where applicable.

## Minimum Contract Values

A future first slice should define value types equivalent to:

- `EditorPreviewSessionId`
- `EditorPreviewRequestId`
- `EditorPreviewDocumentKind`
- `EditorPreviewStatus`
- `EditorPreviewFrameFormat`
- `EditorPreviewFrameDescriptor`
- `EditorPreviewCameraState`
- `EditorPreviewResourceRef`
- `EditorPreviewDiagnostic`
- `EditorPreviewHitRecord`
- `EditorPreviewSelectionRecord`
- `EditorPreviewTransformFeedback`
- `EditorPreviewPlaybackFeedback`
- `EditorPreviewSnapshot`
- `EditorPreviewCounters`

Names may change during implementation, but the responsibilities must not.

## Candidate First Slice

This gate recommends, but does not approve, the first implementable slice:

| ID | Work item | Acceptance direction |
| --- | --- | --- |
| EPV-001 | Session lifecycle contract | start/update/stop returns explicit status and stale-session failure |
| EPV-002 | Frame/headless output descriptor | host reports format, size, status, and diagnostics into caller-owned outputs |
| EPV-003 | Camera state contract | orbit/pan/zoom or equivalent camera state changes preview output identity/status |
| EPV-004 | Resource diagnostics bridge | missing/stale/type-mismatch refs report bounded diagnostics without mutation |
| EPV-005 | Selection/hit feedback records | preview returns bounded hit/selection records; Web overlays remain editor-only |

UI runtime render hook, Scene viewport transform gizmo, Animation playback
target, and cook/package/run smoke should remain later slices unless a reviewer
explicitly approves a narrower staged order.

## Required Future Tests

A future implementation gate must include deterministic fast tests equivalent
to:

- `EditorPreviewHost_StartStopSession_ReturnsExplicitStatus`
- `EditorPreviewHost_RejectsStaleSessionWithoutMutation`
- `EditorPreviewHost_FrameOutputRejectsSmallBufferWithoutMutation`
- `EditorPreviewHost_HeadlessOutputWritesBoundedDiagnostics`
- `EditorPreviewHost_CameraStateChangesFrameSequenceOrStatus`
- `EditorPreviewHost_ResourceRefFailureReportsDiagnostic`
- `EditorPreviewHost_WebOverlayStateDoesNotExportAsRuntimeData`
- `EditorPreviewHost_ShutdownReleasesSessionCapacity`

Focused editor-specific implementation gates should add their own tests:

- UI: runtime layout/render parity against authoritative YuEngine UI output.
- Scene: engine viewport output gated by restore plan and apply-time proof.
- Animation: runtime sample/playback/event feedback against a visible target or
  headless sample output.

## Performance And Safety Constraints

Performance review must check:

- fixed session capacity;
- fixed diagnostic, hit, selection, and counter capacities;
- caller-owned frame/headless outputs or explicitly bounded storage;
- no hidden allocation or unbounded growth on update/frame paths;
- bounded IPC/WebSocket/HTTP payload sizes;
- explicit frame output format and buffer-size failure;
- explicit shutdown and stale-session cleanup;
- no ordinary preview heartbeat/status traffic in durable runtime event streams;
- no UI refresh or Web polling design that forces unbounded engine work.

Security and local-machine rules:

- preview host is local tooling only;
- no remote shell or cloud preview execution;
- no provider token, environment secret, local credential, or raw file content in
  preview diagnostics;
- diagnostics must be bounded and sanitized before Web display.

## Review Routing

Before any implementation task is created:

1. Engine-reference review must confirm mature-engine responsibility separation
   without copying Unity or Unreal API shape.
2. Performance review must approve session/frame/diagnostic capacity and update
   costs.
3. Implementability review must confirm the first slice has enforceable public
   statuses and tests.
4. PM/final gate state must issue explicit `APPROVED_FOR_FIRST_SLICE`.
5. Evidence review is required only if original-game resource, package, scene,
   UI, animation, save, report, or old-runtime facts are proposed as acceptance
   inputs.

## Hard Blocks

The following are blocking violations:

- accepting Web shell, form UI, DOM/CSS, 2D canvas sketches, or static
  screenshots as authoritative runtime preview;
- calling UI, Scene, or Animation editor usable before engine preview host
  output exists for the relevant runtime behavior;
- making Web editor code a dependency of runtime modules;
- making runtime modules depend on editor-only state;
- using fake preview data that cannot pass cook/package/runtime validation;
- bypassing Resource/Package/File validation for preview resource refs;
- bypassing world restore plan or apply-time proof for Scene preview;
- making diagnostics, reports, or oracle outputs own runtime behavior;
- copying old TouhouNewWorld runtime/editor data flow into engine API shape;
- adding a native editor shell fallback to compensate for missing Web/runtime
  preview contracts;
- introducing remote/cloud preview execution.

## Exit Criteria

This gate is ready for review when:

- it is linked from the shared editor preview host plan;
- UI, Scene, and Animation editor plans reference it as the shared preview
  boundary;
- the first slice remains explicitly unapproved until review lanes close;
- the plan keeps Web workspace, local service, and preview host responsibilities
  separate;
- hard blocks prevent Web-only editor pages from being counted as usable editor
  completion.
