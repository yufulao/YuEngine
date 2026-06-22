# YuEngine Runtime Visual Evidence Matrix

Status: current evidence audit only.

This document records what the existing runtime visual work proves today and what it does not prove. It does not approve more RVF-numbered work. The next implementation gate is the runtime asset/data closed loop defined by:

- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`

## Baseline

| Item | Result |
| --- | --- |
| Audited revision | `origin/main @ 883692aff9dbf6ce3c92ea17a702f3a74bc24880` |
| Clean checkout path | `C:\Steam\steamapps\common\TouhouNewWorld\YuEngineCleanGate` |
| Stale `UiWebEditor*` test-source blocker | Fixed by `41708bf Remove stale UI web editor tests` |
| Configure | `cmake --preset windows-fast-gate` PASS |
| Build | `cmake --build --preset windows-fast-gate` PASS |
| Full tests | `ctest --preset windows-fast-gate` PASS, `1261/1261` |

## Evidence Classes

| Class | Meaning | Final asset closed-loop proof |
| --- | --- | --- |
| Programmatic in-memory construction | C++ constructs scene, geometry, material, camera, animation, or proof records directly. | No |
| CPU semantic or PPM oracle | CPU code writes semantic colors or PPM images for deterministic inspection. | No, oracle only |
| GDI viewer | A Windows viewer presents generated pixels through a native window. | No, interactive preview only |
| RenderCore/RHI capture | RenderCore and RHI records flow through runtime submission and capture APIs. | Partial |
| Disk asset load | Runtime reads mesh/material/texture/shader/scene/animation files from File/VFS/Resource and renders them. | Required, missing |

## Gate Command Record

| Command | Result | Notes |
| --- | --- | --- |
| `git worktree add --detach C:\Steam\steamapps\common\TouhouNewWorld\YuEngineCleanGate origin/main` | PASS | Clean checkout for gate proof |
| `cmake --preset windows-fast-gate` | PASS | Configure/regenerate now succeeds |
| `cmake --build --preset windows-fast-gate` | PASS | Full Debug build succeeds |
| `ctest --preset windows-fast-gate` | PASS | `1261/1261` tests passed |
| Focused runtime asset visual tests | FAIL | Not implemented yet; no disk asset closed-loop test exists |

## Matrix

| Layer | Current Status | Evidence Class | Source Files | Tests | Commands | Current Gap |
| --- | --- | --- | --- | --- | --- | --- |
| Camera | PASS for value contracts and runtime visual proof. | Programmatic in-memory construction, CPU semantic/PPM oracle, GDI viewer | `Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCamera.h`, `Tests/RenderCore/RenderCameraTests.cpp`, `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Apps/YuRuntimeVisualCameraTweenViewer/Main.cpp` | `RenderCore` camera tests, `RenderScene_L1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts` | Full `ctest --preset windows-fast-gate` PASS | No camera descriptor is loaded from a scene asset file. Camera tween keyframes are built in C++. |
| Geometry/model | PASS for cube, cylinder, and cone runtime proof. | Programmatic in-memory construction, CPU semantic/PPM oracle, GDI viewer | `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Apps/YuRuntimeVisualCameraTweenViewer/Main.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `RenderScene_L1Vis002CapturesThreePrimitivePlacedSceneThroughRuntimeRoute`, `RenderScene_L1Sample011CapturesFinalRuntimeVisualScene`, `RenderScene_L1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts` | Full `ctest --preset windows-fast-gate` PASS | No mesh asset file exists for cube/cylinder/cone. Mesh data is not loaded, validated, or cooked from disk. |
| Scene placement | PASS for fixed runtime placement and semantic missing-layer checks. | Programmatic in-memory construction | `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Src/YuEngine/RenderScene/Src/RenderSceneMissingLayerDiagnosticRoute.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `RenderScene_L1Vis002CapturesThreePrimitivePlacedSceneThroughRuntimeRoute`, `RenderScene_L1Sample012ReportsExactRuntimeVisualBlocker`, `RenderScene_L1Vis006ReportsExactMissingLayers` | Full `ctest --preset windows-fast-gate` PASS | No scene file references meshes, materials, textures, shaders, camera, and animation together. |
| Material texture slots | PASS for three texture-slot contracts and material proof flags. | Programmatic in-memory construction, CPU semantic oracle, GDI viewer | `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeVisualSceneProofRoute.h`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp`, `Apps/YuRuntimeVisualCameraTweenViewer/Main.cpp` | `RenderScene_RuntimeMaterialBindsThreeTextureSlots`, `RenderScene_RuntimeMaterialRejectsMissingThirdSlot`, `RenderScene_L1Vis003CapturesSharedThreeTextureMaterialSceneThroughRuntimeRoute`, `RenderScene_L1Sample019RendersTexturedGlassEmissiveMetalMaterials` | Full `ctest --preset windows-fast-gate` PASS | Textures are procedural or semantic samples. No texture descriptor/payload file is read from File/VFS/Resource. |
| Shader/pipeline | PASS for current runtime handles, invalid-pipeline diagnostics, and blend-state contract. | Programmatic in-memory construction, RenderCore/RHI capture | `Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgram.h`, `Tests/RenderCore/RenderMaterialTests.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp`, `Src/YuEngine/RHI/Include/YuEngine/RHI/RhiBlendState.h` | `RenderScene_L1Vis001ReportsShaderPipelineMissingLayer`, `RenderScene_RuntimeMaterialReportsInvalidPipeline`, `RenderScene_L1Sample018BlendsTransparentRuntimePanel` | Full `ctest --preset windows-fast-gate` PASS | No shader/program descriptor file is loaded. Shader-like material behavior is still C++ semantic code, not runtime shader asset binding. |
| Animation interpolation | PASS for sampler and frame-context-driven interpolation. | Programmatic in-memory construction | `Src/YuEngine/Animation/Include/YuEngine/Animation`, `Tests/Animation/AnimationRuntimeFoundationTests.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `Animation_RuntimeSamplerInterpolatesDeterministicTransformChannels`, `Animation_RuntimeSamplerUsesFrameContextTime`, `RenderScene_L1Vis004CapturesAnimatedTransformSceneThroughRuntimeRoute` | Full `ctest --preset windows-fast-gate` PASS | No animation clip asset file is loaded, validated, or bound through a scene file. |
| Transform apply | PASS for sampled transform application before RenderScene consumption. | Programmatic in-memory construction | `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Tests/Animation/AnimationRuntimeFoundationTests.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `Animation_RuntimeSamplerAppliesTransformBeforeRenderSceneConsumes`, `RenderScene_L1Vis004CapturesAnimatedTransformSceneThroughRuntimeRoute` | Full `ctest --preset windows-fast-gate` PASS | Transform data is not loaded from disk-backed scene or animation records. |
| RenderScene multi-entity submission | PASS for multiple runtime entities and material variants. | Programmatic in-memory construction, CPU semantic/PPM oracle | `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `RenderScene_L1Vis002CapturesThreePrimitivePlacedSceneThroughRuntimeRoute`, `RenderScene_L1Vis003CapturesSharedThreeTextureMaterialSceneThroughRuntimeRoute`, `RenderScene_L1Sample011CapturesFinalRuntimeVisualScene`, `RenderScene_L1Sample019RendersTexturedGlassEmissiveMetalMaterials` | Full `ctest --preset windows-fast-gate` PASS | RenderScene is not fed by loaded scene/resource records. Runtime data is built directly in test/proof code. |
| RenderCore/RHI capture | PASS for capture API, user-visible extent, and deterministic capture bytes. | RenderCore/RHI capture | `Src/YuEngine/RHI`, `Src/YuEngine/RenderCore`, `Tests/RHI/RhiTests.cpp`, `Tests/RenderCore/RenderCoreTests.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `RHI_CapturePresentedTarget_WritesDeterministicRgba8Bytes`, `RHI_CapturePresentedTarget_WritesUserVisibleResolution`, `RenderCore_FixturePass_ExecutesSampledIndexedPass`, `RenderScene_L1Sample014EmitsUserVisibleCaptureTargetArtifacts` | Full `ctest --preset windows-fast-gate` PASS | RHI capture exists, but no test proves capture from disk-loaded mesh/material/texture/shader/scene assets. Real target absence remains `BlockedByEnv/RhiCaptureTarget` where applicable. |
| Missing-layer diagnostics | PASS for exact semantic layer names in current runtime visual route. | Programmatic fault injection | `Src/YuEngine/RenderScene/Src/RenderSceneMissingLayerDiagnosticRoute.cpp`, `Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp`, `Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp` | `RenderScene_L1Vis006ReportsExactMissingLayers`, `RenderScene_L1Sample012ReportsExactRuntimeVisualBlocker`, `RenderScene_L1Sample013ReportsUserVisibleCaptureResolutionBlocker` | Full `ctest --preset windows-fast-gate` PASS | Missing-layer codes do not yet cover asset file parse, version, dependency, hash, bounds, cook, or load failures. |

## Current Conclusion

Current runtime visual evidence proves that the engine has useful lower-level runtime contracts for camera data, primitive geometry, material slots, animation sampling, transform application, RenderScene submission, RenderCore/RHI capture, blend state, and exact missing-layer reporting.

It does not prove the requested runtime asset/data closed loop. The missing final path is:

`deterministic fixture generator -> disk files -> File/VFS/Resource -> validator -> cook/load -> RenderScene records/resources -> RenderCore/RHI render -> capture`.

CPU semantic pixels, PPM artifacts, and the GDI viewer remain useful development or oracle tools, but they are not acceptable substitutes for the disk asset load and RenderCore/RHI render path.
