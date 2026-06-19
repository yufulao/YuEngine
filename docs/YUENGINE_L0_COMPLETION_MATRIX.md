# YuEngine L0 Completion Matrix

Status: ENG-177A governance document
Baseline: `origin/main@501a460a66dc5d6d0ed7dc313e581a505c85f2d0`
Source plan: `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` sections 10, 11, and 12
Scope: L0 subsystem completion state, commands, allowed skips, forbidden scope, and next action

## 1. Purpose

This document is the required L0 completion matrix from `L0-GOV-001`.
It is a governance document, not an implementation plan and not proof by
itself. A row can only be treated as complete when its required command and
failure model are reproducible.

State values:

- `Done`: reproducible non-hardware command and failure model exist in the
  current repository.
- `FirstSlice`: the boundary exists but still needs a stronger closure command
  or cross-module evidence.
- `BlockedByEnv`: the code path is implemented but needs target hardware,
  driver, OS, codec dependency, or other environment proof before it can be
  called complete.
- `Deferred`: accepted as outside the current L0 closure scope.
- `Rework`: known mismatch with the required boundary.

Current product note:

- This matrix supports an L0 closure-candidate review.
- It does not claim "L0 done for current product" until target production
  hardware paths pass, release validation passes, and bridge audit high-risk
  rows are closed.

## 2. Global Commands

Debug and fast validation:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-dev-gate --output-on-failure
ctest --preset windows-l0-gate --output-on-failure
```

Hardware validation:

```powershell
ctest --preset windows-hardware-smoke --output-on-failure
ctest --preset windows-strict-hardware-smoke --output-on-failure
```

Release validation:

```powershell
cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release-gate --output-on-failure
```

Sample validation:

```powershell
$env:UE_ENGINE_ROOT = '<local Unreal Engine root>'
powershell -ExecutionPolicy Bypass -File Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1 -Configuration Release
```

The sample commands require local Ogg/Vorbis dependencies from the configured
Unreal Engine root. Missing local codec dependencies must be reported as an
environment skip, not as a pass.

## 3. Hardware Skip Grading

| Hardware surface | Allowed skip | Closure blocker | Deferred |
| --- | --- | --- | --- |
| D3D11 device/swapchain/capture/resize | Unsupported GPU, missing feature level, unavailable display session, or CI runner without real D3D11 surface, if the test reports an explicit skip/status | Target production Windows machine is expected to support D3D11 and the row still cannot pass | None for L0 closure candidate; it must be either pass or environment-blocked |
| XAudio2 callback | No output device, XAudio2 initialization unavailable, or strict hardware preset not configured, if explicit unavailable status is reported | Target production Windows machine has an output device and callback smoke cannot pass | Business BGM/SE behavior remains outside L0 |
| XInput gamepad | No gamepad connected or XInput unavailable, if explicit unavailable status is reported | Target production Windows machine has required gamepad hardware and XInput smoke cannot pass | Gameplay command mapping remains outside L0 |
| Sample Ogg/Vorbis dependencies | `UE_ENGINE_ROOT` or required Ogg/Vorbis files are missing and the command reports that dependency gap | Release/sample acceptance requires local sample run and the dependency is available but command fails | Codec ecosystem beyond the sample remains outside L0 |

## 4. Completion Matrix

| Subsystem | Current module/files | Current gate or commit evidence | State | Required closure evidence | Required test command | Allowed skips | Forbidden scope | Owner | Next action |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Platform window/event/native surface | `Src/YuEngine/Platform`, `Tests/Platform` | Platform and HardwareFrameHost tests in `CMakeLists.txt`; ENG-160 hardware frame path landed before L1 work | Done | Create/destroy, invalid desc, focus/resize/close event ordering, opaque surface value, FrameHost consumes events without owning Platform internals | `ctest --preset windows-fast-gate -R "^(Host_|PlatformWindow_|HardwareFrameHost_)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Windowless CI may skip real native window smoke only through explicit status | UI routing, gameplay input, Resource/RenderCore ownership, public backend-native leaks | Platform/Hardware owner | Keep as dependency for bridge audit; do not expand surface without bridge row |
| RHI backend-neutral contract | `Src/YuEngine/Rhi`, `Tests/Rhi` | RHI device/resource/swapchain tests and L0/L1 RHI commits through ENG-160/175 | Done | Capability flags, unsupported states, generation handles, stale handle rejection, deterministic failure statuses | `ctest --preset windows-fast-gate -R "^RHI_" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | HardwareSmoke rows excluded by fast/l0 presets are not part of this software contract row | Scene traversal, material semantics, Resource ownership of backend objects, public D3D11 headers | RHI owner | Freeze public contract before bridge audit; hardware rows below carry target-device proof |
| D3D11 device/swapchain/present/capture | `Src/YuEngine/Rhi`, D3D11 backend tests | D3D11 hardware smoke and render frame smoke targets exist; ENG-160/157/175 evidence added resize/present/capture contracts | BlockedByEnv | Supported machine creates device/swapchain, clears, presents, captures bytes, shuts down with explicit unsupported statuses elsewhere | `ctest --preset windows-hardware-smoke -R "(D3D11|Swapchain|RenderFrame)" --output-on-failure`; strict preset at stage close | Unsupported adapter/display/device may report explicit skip/status | Screenshot/manual proof, backend-native public API leak, blue-screen-only proof | RHI hardware owner | Run hardware smoke on target Windows hardware and record pass/skip reason |
| D3D11 resource/pipeline primitives | `Src/YuEngine/Rhi`, render material/draw/view tests | RHI resource primitives and RenderCore packet tests are present in fast gate | BlockedByEnv | Buffer, texture, sampler, shader, input layout, pipeline, fence/retirement ledger succeed or fail deterministically on real backend | `ctest --preset windows-fast-gate -R "^(RHI_|RenderMaterial_|RenderDrawPacket_|RenderViewPacket_)" --output-on-failure`; hardware D3D11 rows for backend proof | Missing real device can only skip hardware smoke, not value-contract tests | Shader compiler/toolchain expansion, material graph, World/UI/GameAdapter | RHI/RenderCore owner | Keep value tests green; run backend hardware smoke for production claim |
| RHI visible triangle/static mesh/texture sampling | RHI D3D11 smoke, RenderCore draw/material/view packet tests, sample source | Drawable frame and texture sampling evidence from ENG-160F plus sample L0/L1 paths | BlockedByEnv | Visible triangle, indexed static mesh, and texture sampling prove by capture-byte/asserted smoke or explicit environment block | `ctest --preset windows-hardware-smoke -R "(Drawable|Texture|D3D11|Frame)" --output-on-failure`; sample command on supported machine | Missing D3D11 surface or codec dependency may skip only with explicit status | Manual screenshot as primary proof, scene renderer, material graph editor | RHI/RenderCore owner | Keep hardware evidence separate from L1 sample value-contract evidence |
| RHI resize | RHI swapchain resize, HardwareFrameHost resize, RenderCore resize values | ENG-157/160 resize contract landed before L1 batches | BlockedByEnv | Stale target rejection, backbuffer generation change, unsupported resize status, FrameHost resize event path | `ctest --preset windows-fast-gate -R "(Resize|Swapchain|HardwareFrameHost_Resize)" --output-on-failure`; hardware smoke for real swapchain resize | Real swapchain resize may be environment-blocked on headless CI | RenderCore owning swapchain lifecycle, UI resize policy | RHI/Hardware owner | Require target hardware pass before product-done claim |
| RenderCore frame packet/view packet/draw/material/graph execution | `Src/YuEngine/RenderCore`, `Tests/RenderCore`, `Src/YuEngine/RenderScene` value consumers | ENG-152/160/173/175 RenderCore and RenderScene commits | Done | View, draw, material, frame packet, graph skeleton/execution, failure status, no scene dependency in lower RenderCore | `ctest --preset windows-fast-gate -R "^(RenderView|RenderDraw|RenderMaterial|RenderGraph|RenderDrawable|RenderScene_)" --output-on-failure` | Hardware device unavailable is not a skip for value-contract tests | World/UI/GameAdapter dependency in RenderCore, direct D3D11 includes, shader compiler expansion | RenderCore owner | Bridge audit should check RenderScene -> RenderCore direction and lifecycle ownership |
| File/VFS loose read/write and path policy | `Src/YuEngine/File`, `Tests/File` | File forged-path fix and fast gate history before L1 closure | Done | Path normalization, mount priority, loose read/write bounds, async completion status, forged-path rejection | `ctest --preset windows-fast-gate -R "^(File_|AsyncFile|Path|Vfs)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Missing external files are not allowed; tests must use fixtures | Asset semantics, original-game package compatibility, World/GameAdapter | File owner | Keep File policy below Package/Resource; bridge audit covers Streaming file-completion path |
| Package load-plan/staging | `Src/YuEngine/Package`, `Tests/Package` | Package registry/load-plan tests in fast/l0 gates | Done | Manifest/load-plan values, dependency validation, deterministic capacity/status behavior, no old package parser | `ctest --preset windows-fast-gate -R "^Package_" --output-on-failure` | None for fixture tests | Old game package compatibility, File IO policy ownership, Resource lifetime ownership | Package owner | Keep Package as value plan provider only |
| Thread worker / async file-completion | `Src/YuEngine/Thread`, File/Streaming async completion tests | Thread, File, and Streaming tests in windows-l0-gate | Done | Worker lifecycle, completion queue, deterministic shutdown, no sleep-based proof | `ctest --preset windows-fast-gate -R "^(Thread_|File_|Streaming_PackageResourceStaging)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | None for deterministic fixtures | Sleep/manual timing proof, gameplay update scheduling | Thread/File owner | Bridge audit should record File -> Thread ownership direction |
| Resource cache/decode/upload/commit/residency | `Src/YuEngine/Resource`, `Src/YuEngine/Streaming`, `Tests/Resource`, `Tests/Streaming` | ENG-154 Resource decoded payload ownership; ENG-171C resource to RHI handoff; ENG-174/175 Asset fixture consumers | Done | Cache payload, decode plan/result, decoded payload ownership, upload queue, upload completion commit, residency budget, stale handle failures | `ctest --preset windows-fast-gate -R "^(Resource_|Streaming_Resource|Streaming_Package|Asset_)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Hardware RHI unavailable may skip only hardware smoke; value upload fixtures must pass | Resource owning RHI/Audio device lifecycle, report-shaped APIs, old package runtime rollback | Resource/Streaming owner | Feed bridge audit with Resource -> RHI and Resource -> Audio bridge rows |
| Audio mixer/test backend | `Src/YuEngine/Audio`, `Tests/Audio` | Audio deterministic mixer/packet/queue tests and ENG-160E callback bridge work | Done | Test backend/mixer deterministic, voice/source lifecycle, disabled diagnostics neutrality, no business BGM/SE IDs | `ctest --preset windows-fast-gate -R "^Audio_" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Hardware output device skip does not apply to test backend | BGM/SE business IDs, audio scene policy, blocking IO in mixer path | Audio owner | Keep test backend as fast proof; XAudio2 row carries hardware proof |
| XAudio2 callback | `Src/YuEngine/Audio`, hardware smoke tests, sample audio path | ENG-160E Audio PCM queue callback bridge landed; hardware proof is environment-dependent | BlockedByEnv | Supported machine initializes XAudio2, drains PCM queue through callback, reports completion; unavailable device reports explicit status | `ctest --preset windows-hardware-smoke -R "(Audio|XAudio2)" --output-on-failure`; strict preset for target hardware | No output device or XAudio2 unavailable may skip with explicit status | File IO, sleeps, locks, unbounded allocation in callback, gameplay audio events | Audio hardware owner | Run on target machine and record pass/skip reason |
| Audio PCM packet/stream queue | `Src/YuEngine/Audio`, `Tests/Audio` | ENG-160E and L1 AudioScene queue consumers | Done | Caller-owned packet records, bounded stream queue, deterministic drain, explicit overflow/status | `ctest --preset windows-fast-gate -R "(Audio_Pcm|Audio_Stream|AudioScene_)" --output-on-failure` | None for queue fixtures | Device lifecycle ownership in queue, Resource byte parsing inside Audio | Audio owner | Keep as L0 dependency for AudioScene and sample rows |
| Audio Resource import bridge | `Src/YuEngine/AudioResource`, Resource/Asset audio ready records | AudioResource import tests and Asset audio ready tests in fast gate | Done | Decoded audio metadata maps to PCM request without Audio parsing Resource payloads | `ctest --preset windows-fast-gate -R "(AudioResource|Asset_AudioReady|AudioScene_)" --output-on-failure` | None for metadata fixtures | Codec ecosystem expansion, Audio owning Resource payloads | AudioResource/Asset owner | Bridge audit should record Resource/AudioResource/Audio boundaries |
| Input replay/action | `Src/YuEngine/Input`, `Tests/Input` | ENG-160D XInput bridge and ENG-173E/175E command snapshot work | Done | Replayable snapshots, action mapping, deterministic frame values, no platform-native public leak | `ctest --preset windows-fast-gate -R "^(Input_|Input_CommandMapper)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Hardware gamepad unavailable does not skip replay/action fixtures | UI navigation, gameplay command semantics, World/Script callbacks | Input owner | Keep command snapshot as L1 input boundary |
| Win32 input bridge | `Src/YuEngine/Input`, `Tests/Input`, hardware smoke tests | Input bridge tests and HardwareFrameHost input translation tests | Done | Keyboard/mouse/wheel values translate from platform messages, focus lost rejects input, bounded queue behavior | `ctest --preset windows-fast-gate -R "(Input_Bridge|HardwareFrameHost_TranslatesInjectedPlatformInput)" --output-on-failure`; hardware smoke for real Win32 message path | Non-Windows environment may skip hardware smoke; fast injected fixtures must pass | UI routing, text input framework, gameplay commands | Input/Platform owner | Keep platform-native values private; bridge audit to check ownership |
| XInput bridge | `Src/YuEngine/Input`, `Tests/Input`, `Tests/Input/InputHardwareSmokeTests.cpp` | ENG-160D XInput bridge landed; explicit unavailable tests exist | BlockedByEnv | Connected/unavailable/backend-error/invalid-index statuses, no duplicate events, bounded queue; real gamepad smoke on supported machine | `ctest --preset windows-fast-gate -R "Input_BridgeXInput|Input_CommandMapper_XInput" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "XInput" --output-on-failure` | No gamepad or XInput unavailable may skip with explicit status | Gameplay command mapping, UI navigation, silent success on missing hardware | Input hardware owner | Run strict hardware smoke on target gamepad machine |
| Diagnostics/memory/thread cost surfaces | `Src/YuEngine/Diagnostics`, `Src/YuEngine/Memory`, `Src/YuEngine/Thread`, tests | ENG-174D diagnostics counters; existing Memory/Thread/Diagnostics tests | Done | Bounded diagnostics/counters, disabled neutrality, owner/tag memory counters, deterministic thread shutdown, hot-path smoke | `ctest --preset windows-fast-gate -R "^(Diagnostics_|Logging_|Memory_|Thread_)" --output-on-failure`; `ctest --preset windows-dev-gate --output-on-failure` | None for fast fixtures | JSON reports as runtime API, unbounded hot-path logging, diagnostics owning behavior | Quality/Performance owner | Keep diagnostics optional; L1 overlay hook stays tooling plane |
| HardwareFrameHost | `Src/YuEngine/Hardware`, `Tests/Hardware`, Platform/RHI/Input/RenderCore/Audio adapters | ENG-160 full hardware layer and subsequent L1 sample closure; integrated hardware smoke target exists | BlockedByEnv | Initialize/tick/shutdown, injected input, resize, gamepad status, D3D11 integrated frame, explicit unsupported states | `ctest --preset windows-fast-gate -R "^HardwareFrameHost_" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "HardwareFrameHost" --output-on-failure` | Missing D3D11/XAudio2/XInput hardware may skip hardware smoke with explicit status | Owning Platform/RHI/Input/Audio lifecycle internals, gameplay/frame policy | Hardware owner | Target hardware pass required before current-product done claim |
| Engine sample | `Samples/AssetSmokeDemo`, `Tests/Sample` | ENG-171/174/176 sample paths; `YuSampleTests` now cover L1 prep and validation route | BlockedByEnv | Builds from source, no committed generated binaries, proves YuEngine module path, reports audio/gamepad/hardware skip explicitly, handles resize/shutdown | `ctest --preset windows-fast-gate -R "^Sample_L1Vertical" --output-on-failure`; `powershell -ExecutionPolicy Bypass -File Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1 -Configuration Debug`; release variant at stage close | Missing UE Ogg/Vorbis dependency, unsupported D3D11, missing audio device, missing gamepad may skip only with explicit status | Standalone demo path as L1 proof, UI/GameAdapter/gameplay, committed exe/dll/capture proof | Sample owner | Run sample command on supported machine; keep L1 value-contract tests separate from native L0 demo entry |
| Third-party/sample asset hygiene | `Samples/AssetSmokeDemo/README.md`, sample assets, `.gitignore` | ENG-170/171/176 sample cleanup and README provenance | Done | Generated binaries/captures ignored, sample assets intentional, Ogg/Vorbis dependency documented, command documents what it proves and does not prove | `git status --short --ignored Samples/AssetSmokeDemo`; sample README review; `git diff --check -- docs/YUENGINE_L0_COMPLETION_MATRIX.md` | Local third-party dependency absence may skip sample run but not provenance review | Vendored unknown binaries, generated outputs under `Temp`, undocumented asset provenance | Evidence/sample owner | VQ should verify no generated sample outputs are staged |

## 5. L0 Closure Candidate Gate

The current repository may enter L0 closure-candidate review only when all of
these are true:

1. This matrix is committed.
2. `docs/YUENGINE_BRIDGE_AUDIT.md` exists and has no unresolved high-risk L0
   bridge.
3. `windows-fast-gate`, `windows-dev-gate`, and `windows-l0-gate` pass.
4. Release gate passes, or release failure is documented as a blocker.
5. Hardware smoke has no unexpected failures; environment skips are graded by
   the table in section 3.
6. Sample command either passes on a supported machine or reports explicit
   environment blockers.
7. No row depends on UI, GameAdapter, gameplay, old package runtime, manual
   screenshots, manual audio listening, or report/capture artifacts as the
   primary proof.

## 6. Evidence Gaps

| Gap | Classification | Required follow-up |
| --- | --- | --- |
| Target hardware proof for D3D11, XAudio2, XInput, and integrated HardwareFrameHost | `BlockedByEnv` | Run `windows-hardware-smoke` and `windows-strict-hardware-smoke` on the target production Windows machine |
| Release sample command with local Ogg/Vorbis dependency | `BlockedByEnv` | Run `RunAssetSmokeDemo.ps1 -Configuration Release` on a machine with `UE_ENGINE_ROOT` configured |
| Bridge audit high-risk rows | `FirstSlice` | `docs/YUENGINE_BRIDGE_AUDIT.md` exists; keep high-risk bridge rows current and require target hardware proof before current-product done claim |
| L1 governance follow-through | `FirstSlice` | `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`, `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`, and `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` exist; use them as L1 closure inputs rather than re-opening L0 scope |

## 7. Forbidden Expansion Freeze

Until this matrix, the bridge audit, and the L0/L1 sample acceptance documents
are accepted:

- Do not create new broad World, GameAdapter, UI, or gameplay implementation
  tasks.
- Do not use old package runtime compatibility as sample proof.
- Do not route lower modules through report JSON, capture files, manual
  screenshots, or manual audio listening as their required runtime behavior.
- Do not turn hardware skips into `Done` rows without supported-machine command
  evidence.
