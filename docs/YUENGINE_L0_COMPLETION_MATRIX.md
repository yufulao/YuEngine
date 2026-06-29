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

Current checkpoint:

```powershell
ctest --preset windows-hardware-smoke --output-on-failure
```

Result on 2026-06-26: `15/16` executed hardware smoke rows PASS, with one
explicit skip:

- D3D11 device, swapchain, resize, visible triangle, indexed mesh, texture
  sampling, RenderCore drawable frame, RuntimeAssetData device-backed visual
  proof, XAudio2 callback/stream queue, Win32 input bridge, and
  HardwareFrameHost D3D11 integrated frame all pass on the current machine.
- `Input_HardwareBridge_PollsXInputGamepad` is skipped because no connected
  XInput controller is available.

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
| D3D11 device/swapchain/present/capture | `Src/YuEngine/Rhi`, D3D11 backend tests | `ctest --preset windows-hardware-smoke --output-on-failure` passes D3D11 clear/present/capture and swapchain resize rows on the current machine | Done | Supported machine creates device/swapchain, clears, presents, captures bytes, shuts down with explicit unsupported statuses elsewhere | `ctest --preset windows-hardware-smoke -R "(D3D11|Swapchain|RenderFrame)" --output-on-failure`; strict preset at final target audit | Unsupported adapter/display/device may report explicit skip/status on other machines | Screenshot/manual proof, backend-native public API leak, blue-screen-only proof | RHI hardware owner | Keep current hardware command in final audit; do not regress explicit unsupported statuses |
| D3D11 resource/pipeline primitives | `Src/YuEngine/Rhi`, render material/draw/view tests | Current hardware smoke passes primitive resource pipeline, visible triangle, indexed mesh, texture sampling, and RenderCore D3D11 texture capture rows | Done | Buffer, texture, sampler, shader, input layout, pipeline, fence/retirement ledger succeed or fail deterministically on real backend | `ctest --preset windows-fast-gate -R "^(RHI_|RenderMaterial_|RenderDrawPacket_|RenderViewPacket_)" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "D3D11|RenderCore_D3D11" --output-on-failure` | Missing real device can only skip hardware smoke, not value-contract tests | Shader compiler/toolchain expansion, material graph, World/UI/GameAdapter | RHI/RenderCore owner | Keep value tests green and rerun hardware smoke at final target audit |
| RHI visible triangle/static mesh/texture sampling | RHI D3D11 smoke, RenderCore draw/material/view packet tests, sample source | Hardware smoke passes visible triangle, indexed static mesh, texture sampling, and drawable frame texture sampling capture rows | Done | Visible triangle, indexed static mesh, and texture sampling prove by capture-byte/asserted smoke or explicit environment block | `ctest --preset windows-hardware-smoke -R "(VisibleTriangle|IndexedStaticMesh|TextureSampling|Drawable)" --output-on-failure`; sample command on supported machine | Missing D3D11 surface or codec dependency may skip only with explicit status | Manual screenshot as primary proof, scene renderer, material graph editor | RHI/RenderCore owner | Keep hardware evidence separate from L1 sample value-contract evidence |
| RHI resize | RHI swapchain resize, HardwareFrameHost resize, RenderCore resize values | Hardware smoke passes D3D11 swapchain resize invalidation/no-op/invalid-extent and RenderCore resize clear capture rows | Done | Stale target rejection, backbuffer generation change, unsupported resize status, FrameHost resize event path | `ctest --preset windows-fast-gate -R "(Resize|Swapchain|HardwareFrameHost_Resize)" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "Resize|Swapchain" --output-on-failure` | Real swapchain resize may be environment-blocked on headless CI | RenderCore owning swapchain lifecycle, UI resize policy | RHI/Hardware owner | Rerun target hardware smoke at final audit |
| RenderCore frame packet/view packet/draw/material/graph execution | `Src/YuEngine/RenderCore`, `Tests/RenderCore`, `Src/YuEngine/RenderScene` value consumers | ENG-152/160/173/175 RenderCore and RenderScene commits | Done | View, draw, material, frame packet, graph skeleton/execution, failure status, no scene dependency in lower RenderCore | `ctest --preset windows-fast-gate -R "^(RenderView|RenderDraw|RenderMaterial|RenderGraph|RenderDrawable|RenderScene_)" --output-on-failure` | Hardware device unavailable is not a skip for value-contract tests | World/UI/GameAdapter dependency in RenderCore, direct D3D11 includes, shader compiler expansion | RenderCore owner | Bridge audit should check RenderScene -> RenderCore direction and lifecycle ownership |
| File/VFS loose read/write and path policy | `Src/YuEngine/File`, `Tests/File` | File forged-path fix and fast gate history before L1 closure | Done | Path normalization, mount priority, loose read/write bounds, async completion status, forged-path rejection | `ctest --preset windows-fast-gate -R "^(File_|AsyncFile|Path|Vfs)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Missing external files are not allowed; tests must use fixtures | Asset semantics, original-game package compatibility, World/GameAdapter | File owner | Keep File policy below Package/Resource; bridge audit covers Streaming file-completion path |
| Package load-plan/staging | `Src/YuEngine/Package`, `Tests/Package` | Package registry/load-plan tests in fast/l0 gates | Done | Manifest/load-plan values, dependency validation, deterministic capacity/status behavior, no old package parser | `ctest --preset windows-fast-gate -R "^Package_" --output-on-failure` | None for fixture tests | Old game package compatibility, File IO policy ownership, Resource lifetime ownership | Package owner | Keep Package as value plan provider only |
| Thread worker / async file-completion | `Src/YuEngine/Thread`, File/Streaming async completion tests | Thread, File, and Streaming tests in windows-l0-gate | Done | Worker lifecycle, completion queue, deterministic shutdown, no sleep-based proof | `ctest --preset windows-fast-gate -R "^(Thread_|File_|Streaming_PackageResourceStaging)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | None for deterministic fixtures | Sleep/manual timing proof, gameplay update scheduling | Thread/File owner | Bridge audit should record File -> Thread ownership direction |
| Resource cache/decode/upload/commit/residency | `Src/YuEngine/Resource`, `Src/YuEngine/Streaming`, `Tests/Resource`, `Tests/Streaming` | ENG-154 Resource decoded payload ownership; ENG-171C resource to RHI handoff; ENG-174/175 Asset fixture consumers | Done | Cache payload, decode plan/result, decoded payload ownership, upload queue, upload completion commit, residency budget, stale handle failures | `ctest --preset windows-fast-gate -R "^(Resource_|Streaming_Resource|Streaming_Package|Asset_)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Hardware RHI unavailable may skip only hardware smoke; value upload fixtures must pass | Resource owning RHI/Audio device lifecycle, report-shaped APIs, old package runtime rollback | Resource/Streaming owner | Feed bridge audit with Resource -> RHI and Resource -> Audio bridge rows |
| Audio mixer/test backend | `Src/YuEngine/Audio`, `Tests/Audio` | Closed at `origin/main@aee81a39d9d9ee063f9f57bc5bab5137d88cbc9f`: readiness task `453eca90` READY; focused QA task `82548add-9a8a-48a7-adf1-ba837608fd07` first-slice discovery exactly 24 rows, tests `#804` through `#827`, successful `YuAudioTests` focused build, exact execution `24/24`, BGM/SE/SFX/music/business ID scan `0`, clean read-only QA | Done | Test backend/mixer deterministic, voice/source lifecycle, disabled diagnostics neutrality, no business BGM/SE IDs | `ctest --preset windows-fast-gate -R "^Audio_" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Hardware output device skip does not apply to test backend | BGM/SE business IDs, audio scene policy, blocking IO in mixer path | Audio owner | Keep test backend as fast proof; XAudio2 row carries hardware proof |
| XAudio2 callback | `Src/YuEngine/Audio`, hardware smoke tests, sample audio path | Closed at `origin/main@1a1964abbb1ad021d5695ec5ea2e26ee8d5b5f6d`: readiness task `1dec3d24` READY; fast QA task `727479bd-065f-4c6d-9a0f-0cacd2763741` reports callback discovery/execution `18/18`, tests `#828` through `#839` and `#868` through `#873`, successful `YuAudioTests` focused build, exact fast execution `18/18`; hardware QA task `fb347834-96a8-4f5c-913d-d3f354e8478e` reports `windows-hardware-smoke` `2/2`, `windows-strict-hardware-smoke` `2/2`, supported hardware path, no skip, and clean read-only QA; callback cost proof task `39cca45c` at `origin/main@34093cf83ece469c75baad01e8a99b0e426e3d4e` records the single-file `AudioCallbackDeviceWindows.cpp` fix and static scan `0` forbidden callback hot-path operations | Done | Supported machine initializes XAudio2, drains PCM queue through callback, reports completion; unavailable device reports explicit status; callback hot path records bounded atomic pending events and leaves lock/CV merge plus waits in non-callback API paths | `ctest --preset windows-fast-gate -R "^(Audio_Callback(Device_|Desc_|Snapshot_|Completion_|PublicContract_)|Audio_PcmStreamQueueCallback_)" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "^Audio_HardwareCallback_" --output-on-failure`; `ctest --preset windows-strict-hardware-smoke -R "^Audio_HardwareCallback_" --output-on-failure` | No output device or XAudio2 unavailable may skip with explicit status on other machines | Gameplay audio events, sample PCM path, AudioResource and AudioScene ownership | Audio hardware owner | Keep sample PCM path in later row; do not reintroduce callback hot-path blocking or Resource/File parsing |
| Audio PCM packet/stream queue | `Src/YuEngine/Audio`, `Tests/Audio` | Closed at `origin/main@0de7d7076b73d7d716f6d99dca8ac90ac9974247`: readiness task `821f0e53` READY; focused QA task `c80e3337-96db-4521-9c0e-b81d5b882efe` reports `Audio_PcmSamplePacket_` discovery/execution `13/13` at tests `#840` through `#852`, `Audio_PcmStreamQueue_` discovery/execution `15/15` at tests `#853` through `#867`, successful `YuAudioTests` focused build, combined exact execution `28/28`, clean read-only QA | Done | Caller-owned packet records, bounded stream queue, deterministic drain, explicit overflow/status | `ctest --preset windows-fast-gate -R "^(Audio_PcmSamplePacket_|Audio_PcmStreamQueue_)" --output-on-failure` | None for queue fixtures | Device lifecycle ownership in queue, Resource byte parsing inside Audio, callback/hardware/sample policy | Audio owner | Keep as L0 dependency for AudioScene and sample rows |
| Audio Resource import bridge | `Src/YuEngine/AudioResource`, Resource/Asset audio ready records | AudioResource import tests and Asset audio ready tests in fast gate | Done | Decoded audio metadata maps to PCM request without Audio parsing Resource payloads | `ctest --preset windows-fast-gate -R "(AudioResource|Asset_AudioReady|AudioScene_)" --output-on-failure` | None for metadata fixtures | Codec ecosystem expansion, Audio owning Resource payloads | AudioResource/Asset owner | Bridge audit should record Resource/AudioResource/Audio boundaries |
| Input replay/action | `Src/YuEngine/Input`, `Tests/Input` | ENG-160D XInput bridge and ENG-173E/175E command snapshot work | Done | Replayable snapshots, action mapping, deterministic frame values, no platform-native public leak | `ctest --preset windows-fast-gate -R "^(Input_|Input_CommandMapper)" --output-on-failure`; `ctest --preset windows-l0-gate --output-on-failure` | Hardware gamepad unavailable does not skip replay/action fixtures | UI navigation, gameplay command semantics, World/Script callbacks | Input owner | Keep command snapshot as L1 input boundary |
| Win32 input bridge | `Src/YuEngine/Input`, `Tests/Input`, hardware smoke tests | Input bridge tests and HardwareFrameHost input translation tests | Done | Keyboard/mouse/wheel values translate from platform messages, focus lost rejects input, bounded queue behavior | `ctest --preset windows-fast-gate -R "(Input_Bridge|HardwareFrameHost_TranslatesInjectedPlatformInput)" --output-on-failure`; hardware smoke for real Win32 message path | Non-Windows environment may skip hardware smoke; fast injected fixtures must pass | UI routing, text input framework, gameplay commands | Input/Platform owner | Keep platform-native values private; bridge audit to check ownership |
| XInput bridge | `Src/YuEngine/Input`, `Tests/Input`, `Tests/Input/InputHardwareSmokeTests.cpp` | ENG-160D XInput bridge landed; explicit unavailable tests exist | BlockedByEnv | Connected/unavailable/backend-error/invalid-index statuses, no duplicate events, bounded queue; real gamepad smoke on supported machine | `ctest --preset windows-fast-gate -R "Input_BridgeXInput|Input_CommandMapper_XInput" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "XInput" --output-on-failure` | No gamepad or XInput unavailable may skip with explicit status | Gameplay command mapping, UI navigation, silent success on missing hardware | Input hardware owner | Run strict hardware smoke on target gamepad machine |
| Diagnostics/memory/thread cost surfaces | `Src/YuEngine/Diagnostics`, `Src/YuEngine/Memory`, `Src/YuEngine/Thread`, tests | ENG-174D diagnostics counters; existing Memory/Thread/Diagnostics tests | Done | Bounded diagnostics/counters, disabled neutrality, owner/tag memory counters, deterministic thread shutdown, hot-path smoke | `ctest --preset windows-fast-gate -R "^(Diagnostics_|Logging_|Memory_|Thread_)" --output-on-failure`; `ctest --preset windows-dev-gate --output-on-failure` | None for fast fixtures | JSON reports as runtime API, unbounded hot-path logging, diagnostics owning behavior | Quality/Performance owner | Keep diagnostics optional; L1 overlay hook stays tooling plane |
| HardwareFrameHost | `Src/YuEngine/Hardware`, `Tests/Hardware`, Platform/RHI/Input/RenderCore/Audio adapters | Current hardware smoke passes `HardwareFrameHost_D3D11IntegratedFrameRuns`; XInput device polling remains tracked by the separate XInput row | Done | Initialize/tick/shutdown, injected input, resize, D3D11 integrated frame, explicit unsupported states | `ctest --preset windows-fast-gate -R "^HardwareFrameHost_" --output-on-failure`; `ctest --preset windows-hardware-smoke -R "HardwareFrameHost" --output-on-failure` | Missing D3D11/XAudio2 hardware may skip hardware smoke with explicit status; missing XInput stays in the XInput row | Owning Platform/RHI/Input/Audio lifecycle internals, gameplay/frame policy | Hardware owner | Keep XInput hardware blocker separate from integrated D3D11 frame proof |
| Engine sample | `Samples/AssetSmokeDemo`, `Tests/Sample` | ENG-171/174/176 sample paths; `YuSampleTests` now cover L1 prep and validation route | BlockedByEnv | Builds from source, no committed generated binaries, proves YuEngine module path, reports audio/gamepad/hardware skip explicitly, handles resize/shutdown | `ctest --preset windows-fast-gate -R "^Sample_L1Vertical" --output-on-failure`; `powershell -ExecutionPolicy Bypass -File Samples/AssetSmokeDemo/RunAssetSmokeDemo.ps1 -Configuration Debug`; release variant at stage close | Missing UE Ogg/Vorbis dependency, unsupported D3D11, missing audio device, missing gamepad may skip only with explicit status | Standalone demo path as L1 proof, UI/GameAdapter/gameplay, committed exe/dll/capture proof | Sample owner | Run sample command on supported machine; keep L1 value-contract tests separate from native L0 demo entry |
| Third-party/sample asset hygiene | `Samples/AssetSmokeDemo/README.md`, sample assets, `.gitignore` | ENG-170/171/176 sample cleanup and README provenance | Done | Generated binaries/captures ignored, sample assets intentional, Ogg/Vorbis dependency documented, command documents what it proves and does not prove | `git status --short --ignored Samples/AssetSmokeDemo`; sample README review; `git diff --check -- docs/YUENGINE_L0_COMPLETION_MATRIX.md` | Local third-party dependency absence may skip sample run but not provenance review | Vendored unknown binaries, generated outputs under `Temp`, undocumented asset provenance | Evidence/sample owner | VQ should verify no generated sample outputs are staged |

L0-RES-007 docs sync records sample texture/mesh asset path closure at
`origin/main@026f1d06af688ccaa1ff9a421f71ac1ea092cd5a`. Readiness task
`76377a9a` records READY, and focused QA task
`37d47308-4d38-43d0-85cb-d98f9867b6f8` reports Debug and Release
`AssetSmokeDemo` smoke PASS on the current machine with `YuAssetSmokeDemo PASS`,
`YuAssetSmokeDemo L0_ENGINE PASS`, and `YuAssetSmokeDemo L1_PREP PASS`,
`YuSampleTests` focused build PASS, `Sample_L1VerticalPrep_` discovery/execution
`6/6` PASS, dependency discovery-only counts `Streaming_ResourceDecodedTextureBridge_`
`5`, `Streaming_ResourceUpload_.*Texture` `2`, RHI texture/sampler/sampling
`10`, RenderCore texture/material/frame/draw/view `60`, and generated-output
hygiene tracked/staged `0`, untracked `0`, ignored sample/build outputs only.
The matrix still leaves target XInput hardware proof, L0-SAMPLE-004, L1 sample
closure, L0-RHI table closure, manual screenshot/listening proof, broader
RenderScene/L1 visual work, Package/Resource public API expansion, adjacent/full
suites, and broad/full CTest to their own rows.

L0-AUD-001 docs sync records deterministic mixer/test backend closure at
`origin/main@aee81a39d9d9ee063f9f57bc5bab5137d88cbc9f`. Readiness task
`453eca90` records READY, and focused QA task
`82548add-9a8a-48a7-adf1-ba837608fd07` reports first-slice discovery exactly
24 rows, tests `#804` through `#827`, `YuAudioTests` focused build PASS, exact
24-row execution `24/24` PASS, BGM/SE/SFX/music/business ID scan `0`, and a
clean read-only QA workspace. The exact execution reported `0 failed`. The
executed set excluded Callback, PCM packet/stream queue, hardware, sample, and
L1 rows, and the matrix keeps L0-AUD-002, L0-AUD-003, L0-AUD-004, L0-AUD-005,
L0-RES-006, L0-SAMPLE-006, AudioResource, AudioScene, hardware smoke, sample
scripts, manual proof, adjacent/full suites, and broad/full CTest to their own
rows.

L0-AUD-002 docs sync records Audio PCM packet/stream queue closure at
`origin/main@0de7d7076b73d7d716f6d99dca8ac90ac9974247`. Readiness task
`821f0e53` records READY, and focused QA task
`c80e3337-96db-4521-9c0e-b81d5b882efe` reports `Audio_PcmSamplePacket_`
discovery/execution `13/13`, tests `#840` through `#852`,
`Audio_PcmStreamQueue_` discovery/execution `15/15`, tests `#853` through
`#867`, `YuAudioTests` focused build
PASS, combined exact execution `28/28` PASS, and a clean read-only QA
workspace. The exact execution reported
`0 failed`. The executed set excluded callback, hardware, sample, AudioResource,
AudioScene, L1 rows, adjacent/full suites, and broad/full CTest to their own
rows.

L0-AUD-003 docs sync records XAudio2 callback proof closure at
`origin/main@1a1964abbb1ad021d5695ec5ea2e26ee8d5b5f6d`. Readiness task
`1dec3d24` records READY, fast QA task
`727479bd-065f-4c6d-9a0f-0cacd2763741` reports callback discovery/execution
`18/18`, tests `#828` through `#839` and `#868` through `#873`,
`YuAudioTests` focused build
PASS, exact fast execution `18/18` PASS, and a clean read-only QA workspace.
Hardware QA task `fb347834-96a8-4f5c-913d-d3f354e8478e` reports hardware and
strict hardware discovery `2` rows, tests `#874` through `#875`,
`YuAudioHardwareSmokeTests` build
PASS, `windows-hardware-smoke` execution `2/2` PASS,
`windows-strict-hardware-smoke` execution `2/2` PASS, supported hardware path,
and no skip. The exact fast execution reported `0 failed`. The matrix keeps
callback cost, sample PCM path, AudioResource, AudioScene, L1 rows,
adjacent/full suites, and broad/full CTest to their own rows.

L0-AUD-004 docs sync records Audio callback cost proof closure at
`origin/main@34093cf83ece469c75baad01e8a99b0e426e3d4e`. Readiness task
`a1b9ed42` records the pre-fix callback handler lock/CV path as
NEEDS-IMPLEMENTATION, and implementation task `bf2b5bc2` lands the single-file
`Src/YuEngine/Audio/Src/AudioCallbackDeviceWindows.cpp` fix. Focused proof QA
task `39cca45c` reports commit scope limited to that production file, callback
hot-path static scan `0` forbidden operations across `VoiceCallback` lines
`121`-`166` and handler lines `261`-`282`, `YuAudioTests` plus
`YuAudioHardwareSmokeTests` focused builds
PASS, fast callback/bridge execution `18/18` PASS, hardware callback execution
`2/2` PASS, strict hardware callback execution `2/2` PASS,
`git diff --check HEAD^..HEAD` PASS, and clean read-only QA at
`HEAD == origin/main == 34093cf`. The matrix keeps L0-AUD-005 sample PCM path,
AudioResource, AudioScene, L1 rows, adjacent/full suites, and broad/full CTest
to their own rows.

L0-AUD-005 docs sync records sample PCM path closure at
`origin/main@e14869b9138c750152b7e0ea16f466fd4101a8a8`. Readiness task
`5270aafd` records the existing sample path as READY: synthetic audio resource
evidence flows through `AssetAudioReadyRecord` and
`AudioSceneContractQueue::SubmitSourceUpdates` into
`AudioPcmStreamQueueRequest`, with explicit `BackendUnavailable` status.
Focused QA task `e4e6cece` reports `YuSampleTests` build
PASS, `YuAudioSceneTests` build
PASS, focused regex execution only `8/8` PASS with `0 failed` and
`0 skipped/not-run`, and clean read-only QA at
`HEAD == origin/main == e14869b`. The matrix keeps hardware output/listening,
sample scripts/manual proof, L0-SAMPLE-006, AudioResource closure, AudioScene
closure, L1 ASCENE rows, L1-SAMPLE-007, adjacent/full suites, broad/full CTest,
and Render/RHI/World/UI/material/shader/scene/importer/package expansion to
their own rows.

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
| Target XInput gamepad proof | `BlockedByEnv` | Current `windows-hardware-smoke` passes all non-XInput rows and explicitly skips `Input_HardwareBridge_PollsXInputGamepad`; run strict XInput hardware smoke with a connected controller |
| Target hardware-grade sample proof | `BlockedByEnv` | L0-RES-007 records current-machine Debug/Release sample smoke and focused sample value tests; target XInput gamepad and strict hardware proof are governed by the hardware gap above |
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
