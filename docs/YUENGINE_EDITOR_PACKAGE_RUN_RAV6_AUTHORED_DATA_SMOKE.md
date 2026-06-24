# YuEngine Editor Package Run RAV6 Authored Data Smoke

Status: RAV6 Phase 3A implementation slice
Task: #94

## Purpose

This slice verifies that the current Scene Editor, Animation Editor, and UI
Editor workflow outputs are not treated as terminal editor-only rows. The
bridge validates real outputs from:

- `BuildSceneEditorUsableWorkflowSurface`
- `BuildAnimationEditorTimelineWorkflow`
- `BuildUiEditorDesignInspectorWorkflowSurface`

After those outputs are accepted, the bridge consumes the existing
RuntimeAsset deterministic import/cook result and calls
`RunRuntimeAssetPackageArtifactProductCommand`. That product command reads a
Package artifact through File/VFS, rebuilds Package registry state, resolves
the load plan, runs the RuntimeAsset packaged entrypoint, and reaches the
RuntimeApp frame loop.

## Acceptance

- Scene document evidence must include a committed workflow ledger and staged
  transform output.
- Animation evidence must include sampled timeline track rows and selected
  clip/key feedback from Preview Host transform feedback.
- UI evidence must include a valid runtime UI document, staged runtime node
  output, and command ledger.
- RuntimeAsset import/cook evidence must report source and cooked files written
  and validated through the public command result.
- The package/run step must use
  `RunRuntimeAssetPackageArtifactProductCommand`; callers do not provide a
  pre-resolved `PackageLoadPlan`.
- Missing editor evidence returns before the Package command is invoked.
- Missing package artifact returns File/VFS as the first product-run layer and
  does not execute the packaged entrypoint.

## Executable Gate

```powershell
ctest --preset windows-fast-gate -R "EditorPackageRun_" --output-on-failure
```

Related confidence:

```powershell
ctest --preset windows-fast-gate -R "(SceneEditorWorkflow_|AnimationEditorWorkflow_|UiEditorWorkflow_|RuntimeAssetData_ProductRunCommand)" --output-on-failure
```

Full gate:

```powershell
git diff --check
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
git diff --cached --check
git show --check --format=short HEAD
```

## Boundaries

This is not release delivery tooling, full game content, original package
compatibility, external authoring bridge, or final UI document runtime
serialization. The current bridge proves authored editor workflow outputs gate
entry into the existing RuntimeAsset/File/Package/product-run command path and
reports exact missing layers when the path cannot continue.
