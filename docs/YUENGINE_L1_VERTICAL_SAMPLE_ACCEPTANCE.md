# YuEngine L1 Vertical Sample Acceptance

Status: ENG-178C governance document
Baseline: `origin/main@6718b7401377cf6ce50372ba639d91d168bc8b53`
Aligned documents:

- `docs/YUENGINE_L0_COMPLETION_MATRIX.md`
- `docs/YUENGINE_BRIDGE_AUDIT.md`
- `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`
- `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`

Source plan: `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` sections 11, 12.20,
and 12.21
Scope: L1 vertical sample acceptance, `L1-DIAG-003`, `L1-SAMPLE-001..012`,
ENG-176 evidence, release validation route, docs-only next gates,
runtime visual foundation ladder, deferred or environment blockers, and
forbidden scope
Aligned commits: `2b9734d8b334bf746f531fcca7096bb4031ebb21` for document 3,
`3248a2eed7507f6685823eb8c2d780950d28e057` for document 4,
`73164db` for package/product generic submission ledgers, `9153d14` for
material alpha blend-state bridging, and `6718b74` for exact missing-layer
visual proof crash recovery

## 1. Purpose

This document is the required L1 vertical sample acceptance document from the
section 11 document order. It must stay after
`docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md` and
`docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`.

The document defines how the current project-independent L1 vertical sample is
accepted as a value-contract proof. It does not replace L0 native sample smoke,
does not claim target hardware closure, and does not create new implementation
scope.

## 1.1 L1-GOV-003 Scope Decision

L1-GOV-003 vertical sample scope decision is closed by docs-only governance
evidence at `origin/main@d10b9a97fc29381af5b5336fc04326fbf79c3e1c`.
Upstream scout task `a5a9431d` reports COMPLETE-PASS / SPLIT-READY, and docs VQ
task `b750cf98` reports COMPLETE-PASS after the L1-GOV-002 split. This document
already defines the project-independent sample content, `L1-DIAG-003`,
`L1-SAMPLE-001..012`, required subsystem rows, the runtime visual foundation
route, L0 sample relationship, deferred or environment blockers, and forbidden
scope.

This is a scope decision only. Strict target XInput hardware proof, L0
hardware-grade sample proof, final L1 stop condition, L1 final sample
acceptance, runtime animation broad work, World/GameAdapter/UI/gameplay
implementation, Render/RHI/World/UI/material/shader/scene/importer/package
expansion, and old package runtime proof remain outside this sync.

The current value-contract sample is necessary but not sufficient for L1
runtime visual closure. L1 must also carry a pure runtime visual sample that
does not depend on editor, rejected editor route, UI, or input work.

That visual sample must be built from the shallow-to-deep foundation ladder in
`docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md`. Camera capture, scene
placement, geometry/model records, material texture slots, shader/pipeline
binding, animation interpolation, transform application, RenderScene submission,
RenderCore/RHI draw/capture, and missing-layer diagnostics are separate module
floors. A final camera tween sample does not replace those floors.

When the visual sample claims runtime asset/data loading, it must also satisfy
the runtime asset/data contract tracked by
`docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md` and
`docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`: disk files generated
deterministically, read through File/VFS/Resource, validated/cooked/loaded, and
then rendered through RenderScene, RenderCore, and RHI. CPU helper images or
viewer output do not replace that path.

The L1 vertical sample acceptance intent is:

- prove the sample travels through L1 runtime value contracts rather than
  TouhouNewWorld gameplay or old package runtime paths;
- prove manifest, runtime boot, world/object graph, asset binding, input
  command, render scene, audio scene, serialization, cleanup, diagnostics, and
  validation route rows are covered;
- prove, before L1 closure is called complete, a runtime visual scene can render
  and capture a deterministic multi-object camera-tween frame sequence through YuEngine
  resource, material, scene, render, camera, and RHI paths;
- keep native D3D11, XAudio2, XInput, Ogg/Vorbis, and display/session blockers
  graded by `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`;
- keep debug overlay support as an optional tooling plane, not a runtime
  dependency.

## 2. Acceptance States

| State | Meaning |
| --- | --- |
| `Pass` | The value-contract row has deterministic fast evidence and matches the subsystem boundary from the L1 runtime matrix. |
| `StageClose` | The row has a documented command or route, but final closure still requires stage-close VQ, release validation, or target-machine execution. |
| `BlockedByEnv` | The code path exists, but native hardware, display/session state, local codec dependency, or target OS state must be supplied before the native proof can close. |
| `Deferred` | The behavior is intentionally outside the current L1 vertical sample acceptance scope. |
| `Fail` | The row hides missing hardware as success, depends on forbidden scope, mutates generated or implementation files, or violates the required value boundary. |

Missing hardware, missing codec dependencies, and absent debug overlay UI must
not be rewritten as `Pass`.

## 3. Required Command Rows

| Row | Purpose | Required command | Required evidence | Allowed skip |
| --- | --- | --- | --- | --- |
| Fast L1 vertical sample | Prove all current L1 sample value-contract rows | `cmake --preset windows-fast-gate`; `cmake --build --preset windows-fast-gate --target YuSampleTests YuDiagnosticsTests -- /v:minimal`; `ctest --preset windows-fast-gate -R "^(Sample_L1VerticalPrep_|Diagnostics_OverlayHook|Diagnostics_RuntimeCounters_)" --output-on-failure` | `YuSampleTests` and diagnostics rows pass; no native hardware is required for this row | None |
| Filtered L1 integration | Keep L1 rows consistent with L0/L1 matrix routing | `ctest --preset windows-dev-gate --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Filtered integration passes, or failures are documented as unrelated blockers | Hardware smoke rows are excluded and must not be counted as pass |
| Release L1 sample route | Prove the L1 sample route is not Debug-only | `cmake --preset windows-release`; `cmake --build --preset windows-release --target YuSampleTests -- /v:minimal`; `ctest --preset windows-release-gate -R "^Sample_L1VerticalPrep_" --output-on-failure` | Release `YuSampleTests` rows pass at stage close | No skip for value-contract tests; failure is a closure blocker |
| Native L0 sample route reference | Keep native sample smoke proof in the L0 document | See `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` Debug and Release sample smoke rows | Native sample emits `YuAssetSmokeDemo PASS`, `YuAssetSmokeDemo L0_ENGINE PASS`, and any L1 prep line only as additional evidence | `BlockedByEnv` only through explicit L0 sample acceptance states |
| Runtime visual foundation route | Prove L1 builds visual capability from module floors before the final scene | `ctest --preset windows-fast-gate -R "RenderScene_L1Sample(016|018|019)" --output-on-failure`; `Tools/RunFocusedTests.ps1 -Action List -Regex "GenericRenderSceneSubmission"`; `Tools/RunFocusedTests.ps1 -Action List -Regex "CookedRuntimeVisualProofReportsExactMissingLayers|PackageRunEmitsGeneric|ProductRunCommandReportsGeneric|PackageRunRejectsGeneric|BlendState|AlphaBlend"` | Camera tween keyframes, perspective camera, cube/cylinder/cone placement, textured/glass/emissive/metal materials, alpha blend panel, animation-driven transform update, RenderScene/RenderCore/RHI capture, 15 generic submission rows, 13 post-`6718b74` blocker/blend/package rows, and exact missing-layer diagnostics | `BlockedByEnv` only for target D3D11/display constraints; missing camera, geometry/model, material slots, shader/pipeline, animation interpolation, transform application, RenderScene multi-entity/material table, RenderCore multi-draw, package/product ledger, camera tween sampling, or capture path is `Fail` or named missing-layer blocker |
| Runtime visual scene route | Prove L1 can render a concrete runtime scene without editor or input after the foundation route | `RenderScene_L1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts`, `RenderScene_L1Sample018BlendsTransparentRuntimePanel`, `RenderScene_L1Sample019RendersTexturedGlassEmissiveMetalMaterials`, RuntimeAssetData generic submission rows, package/product ledger rows, and `RuntimeAssetData_CookedRuntimeVisualProofReportsExactMissingLayers` | Captures a bounded camera-tween frame set for cube/cylinder/cone with deterministic placement, runtime transform/material proof, textured/glass/emissive/metal materials, blend evidence from `9153d14`, package/product ledger evidence from `73164db`, exact missing-layer no-crash proof from `6718b74`, and explicit status/diagnostics | `BlockedByEnv` only for target D3D11/display constraints; missing foundation floor from `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md` is `Fail` or named missing-layer blocker |
| Docs-only governance lane | Prove this document did not change implementation scope | `git diff --name-status -- docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`; `git diff --check -- docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` | Only this document changes and whitespace is clean | None |

## 4. ENG-176 Evidence Map

| Evidence | Commit | Acceptance contribution |
| --- | --- | --- |
| Deterministic object graph | `d11a74e7666dc2a9ba724d78d5926f2d52d49ee0` | Adds deterministic object/component/transform sample evidence for `L1-SAMPLE-003` |
| Asset, render, audio route closure | `f4b3495` | Adds texture/audio asset binding, RenderScene submit, AudioScene submit, lifecycle route evidence for `L1-SAMPLE-004`, `L1-SAMPLE-006`, and `L1-SAMPLE-007` |
| Diagnostics and validation route | `ce88589` | Adds optional diagnostics overlay hook and Debug/Release/Fast validation route evidence for `L1-DIAG-003` and `L1-SAMPLE-010` |
| Serialize, reload, cleanup proof | `501a460a66dc5d6d0ed7dc313e581a505c85f2d0` | Adds sample state roundtrip and active-record cleanup proof for `L1-SAMPLE-008` and `L1-SAMPLE-009` |

These commits are evidence inputs, not a reason to bypass the command rows in
section 3.

## 5. L1 Vertical Sample Acceptance Matrix

| ID | Acceptance target | Current evidence | State | Required test or command | Deferred / env blocker | Forbidden scope |
| --- | --- | --- | --- | --- | --- | --- |
| `L1-DIAG-003` | Debug overlay hook proposal is optional tooling | `RuntimeDiagnosticsOverlayHook` rejects runtime dependencies and keeps disabled equivalence | Pass | `ctest --preset windows-fast-gate -R "^(Diagnostics_OverlayHook|Diagnostics_RuntimeCounters_)" --output-on-failure` | Visual overlay UI is deferred tooling | Debug overlay required for correctness, report JSON as runtime API, unbounded logging |
| `L1-SAMPLE-001` | Synthetic scene manifest is project-independent | `L1VerticalSamplePrep` builds fixed manifest values and avoids old package runtime parsing | Pass | `ctest --preset windows-fast-gate -R "^Sample_L1VerticalPrep_" --output-on-failure` | Native asset smoke remains in L0 acceptance | Old package runtime, product scene file parsing, gameplay scene content |
| `L1-SAMPLE-002` | Runtime boots and creates world through fixed-frame loop | RuntimeApp and sample prep create runtime/world records and expose fixed-frame counts | Pass | `Sample_L1VerticalPrep_BuildsManifestAndSubmitPrep` | Target hardware failures stay in lower L0 rows | Gameplay update policy, UI/title behavior, old runtime service state |
| `L1-SAMPLE-003` | Object/component/transform graph is deterministic | `Sample_L1VerticalPrep_DeterministicObjectGraphHasStableSlots` covers stable slots and transform snapshot | Pass | `ctest --preset windows-fast-gate -R "^Sample_L1VerticalPrep_DeterministicObjectGraphHasStableSlots$" --output-on-failure` | None for value-contract fixtures | Actor behavior lifecycle, script callbacks, physics or hierarchy policy |
| `L1-SAMPLE-004` | Texture/audio handles bind through Asset Manager | Sample route uses runtime asset handles and keeps lower Resource/Audio ownership outside Asset | Pass | `Sample_L1VerticalPrep_RoutesAssetRenderAudioAndLifecycle` | Native texture/audio device proof stays in L0 acceptance | Asset owning RHI/Audio device lifecycle, game-specific asset IDs, old package compatibility |
| `L1-SAMPLE-005` | Input affects sample only through runtime command values | Sample prep records input command snapshot values and current runtime matrix maps Input command tests | Pass | `Sample_L1VerticalPrep_BuildsManifestAndSubmitPrep`; `Input_CommandMapper_*` through focused L1 route | Real XInput device proof is L0 `BlockedByEnv` | UI navigation policy, text input framework, gameplay command execution |
| `L1-SAMPLE-006` | RenderScene submits RenderCore/RHI-facing frame values | Sample route submits through L1 RenderScene contract queue without D3D11 public exposure | Pass | `Sample_L1VerticalPrep_RoutesAssetRenderAudioAndLifecycle`; `RenderScene_*` through focused L1 route | Real D3D11 frame proof stays in L0 sample/hardware rows | D3D11 includes in public L1 headers, material graph, UI renderer, editor viewport |
| `L1-SAMPLE-007` | AudioScene submits audio or explicit unavailable status | Sample route submits through L1 AudioScene contract queue and keeps backend unavailable status explicit | Pass | `Sample_L1VerticalPrep_RoutesAssetRenderAudioAndLifecycle`; `AudioScene_*` through focused L1 route | Real XAudio2 callback/output proof stays in L0 sample/hardware rows | BGM/SE project service IDs, UI volume menu, blocking IO, device lifecycle ownership |
| `L1-SAMPLE-008` | Sample state roundtrips through value streams | `Sample_L1VerticalPrep_RoundTripsStateAndCleansActiveRecords` covers serialize/reload proof | Pass | `ctest --preset windows-fast-gate -R "^Sample_L1VerticalPrep_RoundTripsStateAndCleansActiveRecords$" --output-on-failure` | File/profile persistence policy remains outside Serialize core | File/Package dependency inside Serialize, old save compatibility, object construction by Serialize |
| `L1-SAMPLE-009` | Shutdown and cleanup leave no active records | Cleanup proof covers world/runtime/resource/asset active records or explicit unavailable states | Pass | `Sample_L1VerticalPrep_RoundTripsStateAndCleansActiveRecords` | Native device shutdown proof stays in L0 sample/hardware rows | Hidden partial cleanup, generated-output proof, sample-owned lower lifecycles |
| `L1-SAMPLE-010` | Debug/Release/Fast validation route is documented | Fast route passes `13/13`; Release `YuSampleTests` route passes `6/6`; native Debug/Release sample scripts print accepted PASS lines with explicit `gamepad=graded_skip` | StageClose | Fast command row in section 3; Release command row in section 3; native L0 sample route from `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` | Strict XInput hardware proof remains `BlockedByEnv`; generated outputs must remain ignored | Treating Debug-only evidence as complete, missing release command, debug overlay as runtime dependency |
| `L1-SAMPLE-011` | Runtime visual scene captures a deterministic multi-object camera-tween sequence | RVF-016/018/019, RuntimeAssetData generic RenderScene submission rows, `73164db` package/product generic submission ledgers, `9153d14` material alpha blend-state rows, and `6718b74` exact missing-layer fix provide checked-in runtime visual evidence | Pass | `ctest --preset windows-fast-gate -R "RenderScene_L1Sample(016|018|019)" --output-on-failure`; `Tools/RunFocusedTests.ps1 -Action List -Regex "GenericRenderSceneSubmission"`; `Tools/RunFocusedTests.ps1 -Action List -Regex "CookedRuntimeVisualProofReportsExactMissingLayers|PackageRunEmitsGeneric|ProductRunCommandReportsGeneric|PackageRunRejectsGeneric|BlendState|AlphaBlend"`; hardware RuntimeAssetData D3D11 smoke on target machine | Target D3D11/display can be `BlockedByEnv` only on unsupported machines; current machine passes the device-backed RuntimeAsset visual row | Editor/rejected editor route/UI/input dependency, standalone D3D sample bypass, manual-only screenshot proof, product gameplay scene, sample-only animation math presented as animation runtime |
| `L1-SAMPLE-012` | Runtime visual blocker report names the missing layer exactly | Missing-layer diagnostics exist for camera, geometry/model, material slots, shader/pipeline, scene placement, animation, transform application, RenderScene, RenderCore/RHI, capture, resource resolution, and camera tween sampling; `6718b74` confirms the exact missing-layer visual proof no longer crashes while reporting diagnostics | Pass | Same route as `L1-SAMPLE-011`; `RuntimeAssetData_CookedRuntimeVisualProofReportsExactMissingLayers`; on failure report one exact layer such as camera, geometry/model path, material texture slots, shader/pipeline, scene placement, animation interpolation, transform application, RenderScene multi-entity, RenderCore multi-draw, RHI capture, camera tween sampling, or resource resolution | None for semantic blockers | Hiding missing visual layer behind L0/L1 value pass, generic "not supported" without owner layer |

## 6. Relationship To L0 Sample Acceptance

`docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` owns native sample smoke and hardware
environment grading. This document owns the L1 value-contract sample.

The boundary is:

- L1 sample rows may use explicit unavailable statuses from lower modules;
- L1 value tests must not require D3D11, XAudio2, XInput, Ogg/Vorbis, or a
  display session;
- L0 native sample smoke must prove native output separately through the Debug,
  Release, HardwareSmoke, and strict HardwareSmoke rows from the L0 sample
  acceptance document;
- an L1 prep line printed by `YuAssetSmokeDemo` is additional evidence only and
  does not replace the L1 fast/release value-contract command rows;
- the runtime visual scene route is an L1 closure requirement and must not be
  deferred to editor work, UI work, rejected editor route preview work, or input work;
- the runtime visual scene route must consume the foundation ladder from
  `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md` rather than appearing as a
  one-off demo.

## 7. Deferred And Environment Blockers

| Blocker | Classification | Effect on L1 vertical sample acceptance | Required follow-up |
| --- | --- | --- | --- |
| Target D3D11 device/swapchain/draw proof | `PASS on current target` | L1 RenderScene and RuntimeAsset device-backed rows have current hardware evidence; unsupported machines must still report explicit skip/status | Keep hardware/sample commands from `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` in final audit |
| Target XAudio2 callback/output proof | `PASS on current target` | L1 AudioScene may pass with explicit unavailable status, and backend callback proof currently passes in hardware smoke and native sample output | Keep hardware audio smoke on the final target machine |
| Target XInput gamepad proof | `BlockedByEnv` | L1 Input mapping fixtures can pass without connected hardware, but real gamepad proof stays L0 proof | Run XInput hardware smoke with a connected target controller; absent controller evidence must remain `BlockedByEnv` |
| Ogg/Vorbis local dependency | `PASS on current target; BlockedByEnv elsewhere` | L1 value tests are not blocked; native sample command has current-target evidence but may be environment-blocked on another machine | Keep `UE_ENGINE_ROOT` and Ogg/Vorbis dependency checks in the final target audit |
| Release value-contract route | `PASS` | Fast and release `Sample_L1VerticalPrep_` rows pass; native Debug/Release sample scripts pass with explicit `gamepad=graded_skip` | Keep Release L1 sample route in final audit |
| Runtime visual foundation route | `PASS` | Camera capture, scene placement, geometry/model, material/texture, shader/pipeline, animation interpolation, transform application, render submission, capture, blend, material semantics, generic submission, package/product ledger, and diagnostics floors have focused tests | Keep RVF and RuntimeAsset visual/product-run focused commands in final audit |
| Runtime visual scene route | `PASS` | Cube/cylinder/cone visual sample passes after the foundation route through camera-tween, RuntimeAssetData device-backed evidence, package/product generic submission ledgers, and exact missing-layer no-crash diagnostics | Keep `L1-SAMPLE-011/012` successor rows in final audit |
| Visual debug overlay implementation | `Deferred` | Hook proposal/status is accepted; actual overlay UI is outside this document | Create a later tooling-only task if needed, without runtime dependency |

## 8. Docs-Only Next Gates

After this document lands, the next governance gates are:

1. `ENG-178VQ` verifies required documents 3/4/5 together.
2. `ENG-178VQ` checks section 11 order, section 12 coverage, docs-only scope,
   and consistency with ENG-176/177 evidence.
3. The final L0/L1 stop-condition audit may continue with the current blocker
   reduced to strict XInput target hardware proof.
4. Stage-close validation may run full fast, release, hardware, strict hardware,
   and sample smoke routes when VQ or the coordinator needs stronger evidence.

No new broad implementation lane is implied by this document. The narrow
`L1-VIS-001..006` foundation route and `L1-SAMPLE-011/012` runtime visual route
now have checked-in evidence. Final closure still needs strict XInput target
hardware proof or an explicit terminal policy accepting that environment
blocker.

## 9. Forbidden Scope

The L1 vertical sample acceptance path must not:

- introduce UI, GameAdapter, gameplay, or product-specific scene behavior;
- require editor, rejected editor route, UI, or input work to satisfy the pure runtime visual
  scene sample;
- claim the final visual sample is valid without first proving or explicitly
  blocking camera, scene placement, geometry/model, material texture slots,
  shader/pipeline, animation interpolation, transform application, render
  submission, capture, blend, material semantics, camera tween sampling, and
  missing-layer diagnostics;
- parse or rely on old package runtime compatibility as sample proof;
- expose D3D11, XAudio2, XInput, Win32, or other backend-native types through
  public L1 headers;
- require report JSON, manual screenshots, manual audio listening, or a debug
  overlay for runtime correctness;
- treat generated capture files as sufficient by themselves; visual sample
  captures must be emitted by a checked-in runtime command with deterministic
  status and missing-layer diagnostics;
- require committed executable, DLL, log, capture, build, or generated output
  artifacts;
- edit source, CMake, tests, scripts, sample assets, or generated files for the
  governance-doc lane that owns this document;
- treat missing hardware, missing codec dependencies, or absent visual overlay
  as `Pass`.

## 10. Acceptance Checklist

`docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` is accepted when:

1. The document is committed after `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`
   and `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`.
2. It covers `L1-DIAG-003` and `L1-SAMPLE-001..012`.
3. It maps ENG-176 evidence to each L1 vertical sample row.
4. It defines Fast, filtered integration, Release, native L0 reference, and
   docs-only governance command rows.
5. It separates L1 value-contract sample proof from native L0 sample smoke and
   hardware environment proof.
6. It lists deferred, stage-close, and environment blockers without rewriting
   them as pass.
7. It requires the runtime visual foundation ladder for L1 closure before the
   final scene route.
8. It requires a pure runtime visual scene route for L1 closure and does not
   defer that route to editor, rejected editor route, UI, or input work.
9. It preserves the forbidden scope from the L0 matrix, bridge audit, L1
   runtime matrix, and L0 sample acceptance document.
10. `ENG-178VQ` verifies this document together with
   `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md` and
   `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`.
