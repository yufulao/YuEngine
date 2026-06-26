# EDITOR-GATE-001: Runtime Preview Host

Status: Proposed architecture/reference gate
Requested decision: `REFERENCE_GATE_REVIEW`
Current decision: `REVIEW_ONLY_NOT_IMPLEMENTATION_APPROVED`
Owner: Architecture
Task: #70; #YuPart task #38
Source baseline: `5dabfae`
Current review baseline: `7ec26be`
Related plans:

- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_ANIMATION_EDITOR_PLAN.md`
- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
- `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
- `docs/YUENGINE_EDITOR_DEPENDENCY_CHAIN_NO_BUILD_LIST.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`

## Purpose

Define the shared editor runtime preview host boundary before UI, Scene, and
Animation editor work claims usable editor progress.

This gate is architecture and reference guidance only. It does not authorize
implementation and does not create a native editor application. Deprecated rejected editor route,
rejected form layout, 2D canvas sketches, or static screenshots must not become
authoritative runtime preview. rejected editor route is retained only as deprecated historical
wording.

The preview host is the bridge between editor tooling and YuEngine
runtime capability:

```text
editor-host commands
-> local editor service
-> isolated engine preview host session
-> runtime validation / resource resolution / frame or headless output
-> bounded status, diagnostics, hit, selection, and frame records back to editor tooling
```

## Layer

L7 editor tooling over approved runtime modules.

The preview host may call approved engine runtime surfaces to build an isolated
preview session. It must not become a lower-layer dependency and must not make
shipped runtime modules depend on editor-only code. Rejected editor route code
must not be a runtime dependency.

## L0/L1 Boundary

L0/L1 completion is a prerequisite, not this gate's acceptance result.

Passing file/resource/package value tests, RHI D3D11 capture-byte tests,
RenderCore fixture tests, RenderScene packet-value tests, or isolated
AssetSmokeDemo screenshots does not prove editor preview capability. Those
results prove the lower layers are available enough to start this gate.

This gate becomes meaningful only when a preview-host session can compose those
lower layers into an engine-owned visual or headless preview output. For Scene
preview, that means at minimum a multi-object, material/texture, camera-driven
frame sequence through YuEngine runtime paths rather than a standalone sample.

## Runtime Asset v0 Input Boundary

The RuntimeAsset v0 input for this gate is the current L1 state recorded as
`RUNTIME_ASSET_MODULE_SLICE_IMPLEMENTED_WITH_PRODUCTION_GAPS`: generated runtime
files are read through File/VFS, `YuRuntimeAsset`, Resource, Asset, RenderScene,
RenderCore, and RHI, and the focused `RuntimeAssetData_*` tests prove the first
disk-backed cube/cylinder/cone closed loop.

That v0 state is enough to define the preview-host MVP interface. It is not
enough to declare the production asset contract complete. The preview host may
consume loaded runtime files, Resource/Asset handles, RenderScene records,
RenderCore/RHI frame output, and the RuntimeAssetData status vocabulary. It must
not define source asset formats, choose cook/package ownership, invent import
metadata, or fill gaps with editor-only fixture structures.

| Neighbor task | Interface to this gate |
| --- | --- |
| #36 Runtime asset contract v0 production-gap closure | owns source/runtime file contract gaps, texture payload consumption, shader program ownership, disk animation sampling, and production scene loader output API |
| #37 UE editor asset/import/preview boundary audit | supplies responsibility-separation evidence only; this gate must not copy Unreal API shape, asset database behavior, or editor extensibility |
| #39 Resource Browser + import/cook/diagnostics scope | owns editor-facing browser/import settings UX and diagnostic display shape; this gate only exposes resource preview refs/statuses |
| #40 Scene/Animation/UI dependency chain and no-build-yet list | owns editor dependency ordering and explicit no-build-yet items; this gate only defines the shared preview host floor they later depend on |

If #36 records a production gap for the canonical scene route, the MVP preview
host must report that exact gap as a blocker or partial status. It must not route
around the gap through in-memory fixture structs, deprecated editor mock data,
screenshots, or report/oracle output.

## Reference Boundary

Reference engines are used only for responsibility separation:

- Unity and Unreal both treat editor surfaces, runtime preview viewports,
  resource validation, and play/simulate sessions as separate responsibilities.
- YuEngine should borrow that separation: editor tooling owns panels and editor
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
- local editor-host protocol shape between local editor service and editor
  tooling;
- no-mutation failure semantics for invalid requests and insufficient output
  capacity;
- explicit session shutdown behavior and stale-session rejection.

The host may expose runtime-derived data to editor tooling. Editor overlays such
as selection boxes, transform handles, grid, labels, and timeline markers are
editor-only and never authoritative runtime output.

## Does Not Own

This gate does not own:

- editor panel layout, hierarchy, inspector, resource browser, timeline,
  shortcuts, drag/drop, undo/redo, or workflow behavior;
- a native editor app, immediate-mode editor fallback, or deprecated C++ rejected editor route editor shell;
- forbidden rejected form layout/rejected document tree/canvas as authoritative game/editor preview;
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

1. Editor tooling is a client, not preview authority.
2. Local editor service routes load/save/validate/cook/preview commands. It must
   not own editor route panels or runtime behavior.
3. Preview host sessions are isolated from the normal game runtime and must be
   explicitly started and stopped.
4. Runtime data is the source of truth. Editor-only state must be sidecar data.
5. Preview resource resolution must use approved Resource/Package/File paths or
   return explicit diagnostics.
6. Preview may consume RuntimeAsset v0 loaded graphs and statuses, but must not
   define asset source formats, import metadata, cook/package ownership, or
   production loader gaps.
7. Preview cannot bypass approved restore/preflight/proof gates when Scene or
   World data is involved.
8. UI preview must use YuEngine UI runtime output, not rejected layout.
9. Animation preview must use runtime clip/player/sample/event output, not only
   timeline drawing.
10. Scene preview must use engine-rendered or engine-headless scene outputs, not
    a standalone deprecated rejected editor route canvas mock.
11. L0/L1 pass, RHI fixture pass, RenderCore fixture pass, and isolated sample
    screenshots must never be reported as Preview Host pass.
12. Failures must be explicit and no-mutation where applicable.

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

#YuPart task #65 implements the first bounded `YuPreviewHost` target described
in `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`. It realizes the value/session,
RuntimeAsset graph/ref diagnostics, headless/frame output, and canonical
RenderScene/RenderCore/RHI capture portions below. It remains a lower engine
preview host slice, not `EditorRuntimePreviewHost`, a native editor app, local
editor service, Resource Browser UX, or any Scene/Animation/UI editor panel.

The historical candidate table was:

| ID | Work item | Acceptance direction |
| --- | --- | --- |
| EPV-001 | Session lifecycle contract | start/update/stop returns explicit status and stale-session failure |
| EPV-002 | Frame/headless output descriptor | host reports format, size, status, and diagnostics into caller-owned outputs |
| EPV-003 | Camera state contract | orbit/pan/zoom or equivalent camera state changes preview output identity/status |
| EPV-004 | RuntimeAsset v0 resource diagnostics bridge | missing/stale/type-mismatch RuntimeAsset/Resource/Asset refs report bounded diagnostics without mutation |
| EPV-005 | Selection/hit/transform feedback records | preview returns bounded hit, selection, and transform feedback records; editor overlays remain editor-only |
| EPV-006 | Canonical scene proof contract | fixed-seed cube/cylinder/cone scene loaded from RuntimeAsset v0 files, with one three-texture material, object rotation, orbit camera, and bounded frame capture set, reports pass or exact #36 production-gap blocker |

Resource Browser/import UI, UI runtime render hook, Scene viewport transform
gizmo, Animation playback target, and cook/package/run smoke should remain later
slices unless a reviewer explicitly approves a narrower staged order.

## Required Future Tests

A future implementation gate must include deterministic fast tests equivalent
to:

- `EditorPreviewHost_StartStopSession_ReturnsExplicitStatus`
- `EditorPreviewHost_RejectsStaleSessionWithoutMutation`
- `EditorPreviewHost_FrameOutputRejectsSmallBufferWithoutMutation`
- `EditorPreviewHost_HeadlessOutputWritesBoundedDiagnostics`
- `EditorPreviewHost_CameraStateChangesFrameSequenceOrStatus`
- `EditorPreviewHost_ResourceRefFailureReportsDiagnostic`
- `EditorPreviewHost_rejected editor routeOverlayStateDoesNotExportAsRuntimeData`
- `EditorPreviewHost_ShutdownReleasesSessionCapacity`
- `EditorPreviewHost_CanonicalScene_CapturesCubeCylinderConeOrbitSequence`
- `EditorPreviewHost_UsesRuntimeAssetV0LoadedGraphWithoutDefiningFormats`
- `EditorPreviewHost_ReportsRuntimeAssetProductionGapAsExplicitBlocker`
- `EditorPreviewHost_SelectionTransformFeedbackRejectsInvalidRequestWithoutMutation`

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
- bounded local editor-host protocol payload sizes;
- explicit frame output format and buffer-size failure;
- explicit shutdown and stale-session cleanup;
- no ordinary preview heartbeat/status traffic in durable runtime event streams;
- no UI refresh, polling, or editor-host design that forces unbounded engine work.

Security and local-machine rules:

- preview host is local tooling only;
- no remote shell or cloud preview execution;
- no provider token, environment secret, local credential, or raw file content in
  preview diagnostics;
- diagnostics must be bounded and sanitized before editor display.

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

For #YuPart task #38 specifically, review must also confirm:

- #36 owns RuntimeAsset v0 production-gap closure and this gate only consumes its
  loaded-graph/status output;
- #37 reference findings are used only as boundary evidence;
- #39 owns Resource Browser/import/cook/diagnostic editor UX beyond preview refs;
- #40 owns Scene/Animation/UI dependency ordering and no-build-yet items.

## Hard Blocks

The following are blocking violations:

- claiming L0/L1 plan completion, RHI fixture capture, RenderCore fixture pass,
  RenderScene packet values, or isolated sample screenshots satisfy Preview Host
  or editor visual capability;
- accepting deprecated rejected editor route shell, form UI, rejected document tree/rejected style, 2D canvas sketches, or static
  screenshots as authoritative runtime preview;
- claiming RuntimeAsset v0 smoke closes the production asset contract or lets
  Preview Host define source asset formats;
- calling UI, Scene, or Animation editor usable before engine preview host
  output exists for the relevant runtime behavior;
- making rejected editor route code a dependency of runtime modules;
- making runtime modules depend on editor-only state;
- using fake preview data that cannot pass cook/package/runtime validation;
- using in-memory fixture structs, editor-only mock data, or helper/oracle output
  to bypass loaded runtime files for the canonical scene proof;
- bypassing Resource/Package/File validation for preview resource refs;
- bypassing world restore plan or apply-time proof for Scene preview;
- making diagnostics, reports, or oracle outputs own runtime behavior;
- copying old TouhouNewWorld runtime/editor data flow into engine API shape;
- adding a native editor shell fallback to compensate for missing runtime preview
  contracts;
- introducing remote/cloud preview execution.

## Exit Criteria

This gate is ready for review when:

- it is linked from the shared editor preview host plan;
- it is linked from the RuntimeAssetData plan and L1 runtime asset closed-loop
  gate as an after-asset-v0 consumer, not an asset-contract owner;
- UI, Scene, and Animation editor plans reference it as the shared preview
  boundary;
- it explicitly separates L0/L1 foundation completion from editor visual
  capability acceptance;
- the first slice remains explicitly unapproved until review lanes close;
- the plan keeps editor tooling, local service, and preview host
  responsibilities separate, with rejected editor route retained only as deprecated historical
  wording;
- the task #36/#37/#39/#40 interfaces are named so MVP scope cannot silently
  absorb asset, reference-audit, resource-browser, or editor-chain ownership;
- hard blocks prevent deprecated deprecated editor-route-only editor pages from being counted as usable editor
  completion.
