# YuEngine L0 Sample Acceptance

Status: ENG-178B governance document
Baseline: `origin/main@2b9734d8b334bf746f531fcca7096bb4031ebb21`
Aligned documents:

- `docs/YUENGINE_L0_COMPLETION_MATRIX.md`
- `docs/YUENGINE_BRIDGE_AUDIT.md`
- `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`

Source plan: `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` sections 11, 12.1,
and 12.9
Scope: L0 sample smoke command, Debug/Release/Fast/HardwareSmoke rows,
allowed skips, hardware environment blockers, forbidden scope, evidence
commands, and generated-output policy

## 1. Purpose

This document is the required L0 sample acceptance document from the section 11
document order. It must stay after `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`
and before `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`.

The document defines how the current `Samples/AssetSmokeDemo` path is accepted
as an L0 sample smoke route. It does not create implementation scope, does not
replace module tests, and does not declare the commercial engine complete.

The sample acceptance intent is:

- prove the sample can be built from source or fail with an explicit local
  environment blocker;
- prove the sample uses YuEngine L0 module paths instead of committed generated
  binaries;
- grade D3D11, XAudio2, XInput, Ogg/Vorbis, and display/session blockers
  consistently with the L0 completion matrix;
- keep L1 vertical sample value-contract proof separate from native L0 sample
  smoke proof.

## 2. Acceptance States

| State | Meaning |
| --- | --- |
| `Pass` | The command ran and produced the required deterministic status or output. |
| `BlockedByEnv` | The code path exists, but the local machine lacks a required hardware surface, device, driver, display session, or Ogg/Vorbis dependency, and the command reports that reason explicitly. |
| `Fail` | The command fails without an allowed explicit environment status, mutates forbidden files, relies on generated output, or hides missing hardware as success. |
| `Deferred` | The behavior is outside L0 sample acceptance and belongs to L1 vertical sample acceptance or a later product gate. |

Missing hardware or local codec dependencies must never be rewritten as
`Pass`. Fast value-contract tests must not skip because of missing hardware.

## 3. Required Command Rows

| Row | Purpose | Required command | Required evidence | Allowed skip |
| --- | --- | --- | --- | --- |
| Fast sample value route | Keep sample-facing value contracts deterministic without native hardware | `cmake --preset windows-fast-gate`; `cmake --build --preset windows-fast-gate --target YuSampleTests -- /v:minimal`; `ctest --preset windows-fast-gate -R "^Sample_L1VerticalPrep_" --output-on-failure` | `YuSampleTests` sample rows pass and remain fixture-based; no D3D11/XAudio2/XInput dependency is required for this row | None |
| Debug sample smoke | Build and run the standalone sample from source in Debug | `$env:UE_ENGINE_ROOT = '<local Unreal Engine root>'`; `powershell -ExecutionPolicy Bypass -File Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1 -Configuration Debug` | The command builds `YuAssetSmokeDemo`, runs it, and prints `YuAssetSmokeDemo PASS` plus `YuAssetSmokeDemo L0_ENGINE PASS` with explicit route states | `BlockedByEnv` only for missing Ogg/Vorbis dependency, unavailable D3D11/display, unavailable XAudio2, or unavailable XInput, all with explicit status |
| Release sample smoke | Confirm the sample route is not Debug-only | `$env:UE_ENGINE_ROOT = '<local Unreal Engine root>'`; `powershell -ExecutionPolicy Bypass -File Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1 -Configuration Release` | Same evidence as Debug, produced from Release build outputs | Same as Debug; if local dependency exists and command fails, this is a closure blocker |
| L0 filtered gate | Keep L0 module rows green while sample governance is accepted | `ctest --preset windows-l0-gate --output-on-failure` | L0 filtered tests pass or report only documented unrelated blockers | HardwareSmoke rows are excluded from this preset and are not counted as sample hardware proof |
| Hardware smoke | Prove native D3D11/XAudio2/XInput/HardwareFrameHost paths on a supported machine | `ctest --preset windows-hardware-smoke --output-on-failure` | Hardware rows pass on supported production Windows hardware, or report explicit unavailable status on unsupported machines | `BlockedByEnv` for unsupported adapter/display/device/audio/gamepad state only |
| Strict hardware smoke | Prevent silent hardware absence from being treated as closure | `ctest --preset windows-strict-hardware-smoke --output-on-failure` | Strict D3D11/XAudio2/XInput/HardwareFrameHost rows pass on the target machine selected for closure | May be `BlockedByEnv` before target hardware review; must not be called complete on a machine missing required hardware |
| Generated-output hygiene | Prove the sample command did not require committed generated artifacts | `git status --short --ignored Samples/AssetSmokeDemo`; `git diff --check -- docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` | No generated sample executable, DLL, log, capture, or `Build/` output is staged; this document has clean whitespace | None |

## 4. Required Sample Output

The standalone sample command is accepted only when the command reaches the
sample executable and emits the expected route status lines:

```text
YuAssetSmokeDemo PASS mesh_vertices=36 texture=128x128 bgm=XAudio2/Vorbis seconds=8
YuAssetSmokeDemo L0_ENGINE PASS file_bytes=... decoded_texture=2x2 upload_generation=... render_frames=... input_events=... gamepad=pass|graded_skip audio=pass|graded_skip resize=pass shutdown=pass
```

`gamepad=graded_skip` is acceptable only when no XInput source or controller is
available and the sample reports that state. `audio=graded_skip` is acceptable
only when XAudio2 output or callback completion is unavailable and the sample
reports that state.

`YuAssetSmokeDemo L1_PREP PASS` may appear after later L1 sample work, but it
does not replace the L0 sample output above. L1 vertical sample acceptance owns
the L1-specific rows.

## 5. Allowed Environment Skips

| Surface | Allowed environment state | Closure blocker |
| --- | --- | --- |
| Ogg/Vorbis dependency | `UE_ENGINE_ROOT` is unset or does not point to Unreal Engine Ogg/Vorbis headers, import libraries, and runtime DLLs; configure or script output identifies the missing file | The dependency exists on the validation machine and the sample still fails to configure, build, copy runtime DLLs, or run |
| D3D11 device/display | Adapter, feature level, swapchain, capture, or display session is unavailable and the hardware row or sample reports an explicit unavailable status | Target production Windows machine is expected to support D3D11 and the command cannot clear, present, capture, resize, or shut down |
| XAudio2 callback/output | No output device, XAudio2 unavailable, or callback completion cannot be proven, with explicit `audio=graded_skip` or hardware-smoke status | Target machine has an output device and XAudio2 callback smoke cannot pass |
| XInput gamepad/source | No gamepad connected, XInput unavailable, or controller source unavailable, with explicit `gamepad=graded_skip` or hardware-smoke status | Target machine has a required gamepad and XInput smoke cannot pass |
| Headless or CI environment | Native window/display hardware smoke reports an explicit unsupported status | A supported desktop validation machine cannot create the native surface or sample window |

An allowed skip is not a pass. It records that the current machine cannot close
the native proof. The target hardware closure record must rerun the relevant
row and either pass or report a closure blocker.

## 6. Generated Output Policy

`Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1` must build the sample from
source. The repository must not rely on precommitted sample executables, copied
third-party DLLs, logs, screenshots, or capture images.

Generated outputs include:

- `Samples/AssetSmokeDemo/Build/`;
- copied Ogg/Vorbis DLLs next to `YuAssetSmokeDemo.exe`;
- `FormalOggCapture.bmp`;
- local logs, traces, or capture files produced by sample runs.

These files may exist locally after a run only if they remain ignored and are
not staged. Source-controlled sample assets under `Samples/AssetSmokeDemo/Assets`
remain intentional fixtures, subject to the provenance notes in the sample
README.

## 7. Relationship To L1 Runtime Matrix

`docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md` records L1 value-contract subsystem
state and treats native sample proof as separate stage-close evidence. This
document closes the L0 sample acceptance side of that governance follow-through.

The boundary between this document and L1 vertical sample acceptance is:

- this document owns native sample smoke, hardware environment grading,
  Debug/Release sample command acceptance, and generated-output policy;
- `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` owns `L1-SAMPLE-001..010`,
  `L1-DIAG-003`, project-independent scene manifest acceptance, runtime value
  route acceptance, and final L1 vertical sample stop conditions.

L1 value-contract tests may consume explicit unavailable statuses from L0, but
they must not convert missing D3D11, XAudio2, XInput, or Ogg/Vorbis dependencies
into L0 completion.

## 8. Forbidden Scope

The L0 sample acceptance path must not:

- introduce UI, GameAdapter, gameplay, or product-specific scene behavior;
- use old package runtime compatibility as sample proof;
- expose D3D11, XAudio2, XInput, Win32, or other backend-native types through
  public runtime headers;
- use report JSON, capture files, screenshots, or manual audio listening as the
  primary behavior transport;
- require committed executable, DLL, log, capture, or build output artifacts;
- vendor new third-party binaries without a separate provenance and license
  review;
- edit source, CMake, tests, scripts, sample assets, or generated files for the
  governance-doc lane that owns this document.

## 9. Acceptance Checklist

`docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` is accepted when:

1. The document is committed after `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`.
2. It covers `L0-GOV-003`, `L0-GOV-004`, `L0-GOV-005`, and
   `L0-SAMPLE-001..008`.
3. It defines Debug, Release, Fast, HardwareSmoke, strict HardwareSmoke, sample
   smoke, and generated-output hygiene rows.
4. It grades all allowed sample and hardware skips as `BlockedByEnv`, not
   `Pass`.
5. It keeps generated outputs out of version control.
6. It preserves the forbidden scope from the L0 matrix, bridge audit, and L1
   runtime matrix.
7. `ENG-178VQ` verifies this document together with
   `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md` and
   `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`.
