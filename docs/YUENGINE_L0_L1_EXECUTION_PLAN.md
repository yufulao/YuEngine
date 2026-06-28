# YuEngine L0/L1 Execution Plan

Status: handoff plan for landing team
Owner: 八云紫, 总架构师
Requested: 2026-06-19
Current observed code checkpoint: `d18f1679ebd389ecec506055764602591f5b9ab6`
Scope: progress adjustment, L0 closure plan, L1 runtime-core plan
Canonical entry point: `docs/README.md`
Parent plan: `docs/YUENGINE_LONG_PLAN_TEAM_EXECUTION.md`

## 0. Production Target And Reference Bar

YuEngine is the production engine for a small independent team. It must not be
managed as a toy engine, a technology demo, or a public UE/Unity clone. The
accepted reference bar is the shipped TouhouNewWorld class of native game:

- small native runtime executable and support DLLs;
- no Unity/UE runtime shape as the target architecture;
- shipped-content-scale packed resources used as pressure examples, with
  explicit byte-budget assumptions named per accepted gate;
- package/index/config/shader surfaces that support long-session play;
- 20 hour scale gameplay stability, high performance, and actionable
  diagnostics.

This changes the L0/L1 plan from "prove many feature slices" to "build the
production spine":

```text
Package -> Resource -> RuntimeAsset -> runtime records -> instance mapping
-> Render/Audio/Input/Serialize -> tools -> shipping
```

All new work must prove how it moves this spine toward a shippable native game.
If a feature does not reduce production risk for packed assets, long-session
runtime stability, or deterministic diagnostics, it is not a near-term L0/L1
priority.

## 1. Executive Decision

YuEngine should not continue as a sequence of isolated first-slice gates. The
current project has valuable lower-engine proof, but the next correction is to
turn proof slices into vertical runtime closure.

The immediate adjustment is:

```text
Stop saying: hardware layer is complete
Say instead: L0 hardware-facing lower layer has a closure candidate

Stop expanding: more World bridge slices as proof of engine progress
Start closing: L0 matrix, one real engine sample, then L1 runtime vertical slice

Stop optimizing for: UE/Unity ecosystem breadth
Optimize for: a clean internal commercial engine for one known game team

Stop treating: animation, model, scene, and shader work as independent islands
Start enforcing: asset family work flows through Package/Resource/RuntimeAsset
               and stable asset-internal identity before World/editor binding
```

Focused-first verification is the default for direct-main slices. Use
`Tools/RunFocusedTests.ps1` to list or run named tests, module labels, or labels
derived from changed paths before considering any broad CTest surface. Full
CTest gates are reserved for explicit quality, release, or shared-contract
decisions.

UE and Unity remain reference engines for responsibility boundaries, failure
models, production tooling, and performance discipline. YuEngine should not
copy their public ecosystem, universal plugin surface, editor extension market,
multi-industry platform scope, or generic API breadth.

The local TouhouNewWorld shipped package is the practical production reference:
small native runtime, multi-GB packed content, explicit resource indexes, and
stable long-session play. YuEngine should be narrower than that game's engine
where the team does not need a feature, but not weaker in runtime discipline,
asset packaging, failure semantics, or performance evidence.

## 1.1 Long-Horizon Global Roadmap

The longest execution line is:

1. **L0 production foundation**: platform, RHI, audio, input, File/VFS,
   Package, Resource, RenderCore, diagnostics, memory, and thread proof.
2. **L1 runtime core**: RuntimeApp, FrameContext, Object, Component, Transform,
   World/Scene records, Asset Manager, RenderScene, AudioScene, Input mapping,
   Serialize, Script bridge, and Diagnostics.
3. **Runtime asset production spine**: source/cooked schemas, package indexes,
   hashes, dependency tables, archive budgets, validators, cook/load stages,
   and runtime asset family records.
4. **Product runtime feature set**: model, skeleton, scene, animation,
   material, shader, texture, audio, input, save/config, and long-session
   resource lifecycle.
5. **Production tools**: importer, cooker, validator, package builder, resource
   browser, preview surfaces, performance capture, and crash triage.
6. **Shipping and maintenance**: release packages, patch/migration policy,
   soak tests, hardware matrix, save compatibility, and versioned diagnostics.

Editor work belongs after runtime contracts. Old TouhouNewWorld compatibility
belongs only behind a separate accepted gate. Neither is allowed to shape L0/L1
runtime contracts by default.

## 1.2 Nearest Stage Decision

The nearest stage is RuntimeAsset spine correction, not broad feature growth.
Current scene-animation selected-clip evidence is useful and should close
through VQ, but it does not authorize deeper animation work until the missing
asset-internal target contract is explicit.

The required order for the next RuntimeAsset-adjacent work is:

```text
RuntimeAsset container/family identity
-> package/resource index and dependency table
-> asset-internal scene node / model node / skeleton joint identity
-> animation track/channel binding to target + property
-> Step/Linear interpolation
-> sampled transform application to runtime instance records
-> WorldObject mapping only after instance contracts exist
-> editor/importer authoring surfaces after runtime contracts pass
```

Animation assets must not bind directly to WorldObject, editor object, raw
pointer, display name, or file path. Shader/reflection hardening can continue as
a separate asset-family lane only when it does not replace the missing node and
target identity work.

### 1.2.1 Immediate Detailed Backlog

| ID | Work item | Acceptance |
| --- | --- | --- |
| RTSPINE-001 | Close current scene-animation evidence gate | implementation, QA, docs, VQ, `origin/main`, and matrices agree before any next lane opens |
| RTSPINE-002 | Update production target planning docs | TouhouNewWorld-class native runtime, shipped-content-scale package pressure examples, and no-compatibility policy are written into the active plans |
| RTSPINE-003 | Define asset-internal target identity | PASS at `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`: scene node, model node, and skeleton joint ids are stable bounded output records independent from WorldObject; duplicate id, missing parent, and capacity overflow fail without mutation |
| RTSPINE-004 | Define animation track target binding | PASS at `origin/main@ebe9ea35f531aa40133262b701e5e751f8ed9ccf`: SceneNode animation tracks bind to `target_id` plus property through caller-owned binding records; missing target, unsupported property, and capacity overflow fail without mutation; no world instance, editor object, raw pointer, display name, or file path |
| RTSPINE-005 | Define minimal interpolation | PASS at `origin/main@2bfe7e37d36ca711dd706728f21b1e4caecfd3db` with focused QA at `origin/main@d18f1679ebd389ecec506055764602591f5b9ab6`: Step and Linear sampling at fixed times are deterministic, unsupported interpolation and sample output capacity fail without mutation, exact interpolation rows report `3/3` PASS, and no broad/full CTest was run |
| RTSPINE-006 | Define invalid target failure model | PASS at `origin/main@96e0c024435f670c39ced019ff825b819a6830a3`: target-family mismatch and sample failure diagnostics fail without output mutation; focused QA task `6d02c260-936a-456b-917b-5c2802bbb666` reports isolated clean worktree, focused RuntimeAsset regex `8/8` PASS, exact new rows `2/2` PASS, diff/hygiene/boundary PASS, and no broad/full CTest |
| RTSPINE-007 | Define runtime instance mapping gate | PASS at `origin/main@37a112549190ac2123abcd72b5c688cdfa5b01e5`: asset target records map to caller-owned runtime instance rows for scene entities before any WorldObject/editor binding; focused QA task `6b6baf5f-2381-4b9c-89b1-4411fba53d23` reports exact RuntimeInstanceMapping rows `5/5` PASS and no broad/full CTest |
| RTSPINE-008 | Define package/resource pressure gate | 008A pressure contract defines vocabulary, byte-range policy, hash coverage, mutation contract, and forbidden scope; 008B byte-range/index, 008C Package artifact hash/dependency integrity, 008D File/VFS ranged IO, 008E Resource payload window/reference budget, 008F Package dependency closure/budgeted load plan, 008G RuntimeAsset packaged validation bridge, and 008H RuntimeAsset transaction rollback/proof are PASS; next work covers broader Resource/File/VFS follow-through |

### 1.2.2 RTSPINE-008A Package/Resource Pressure Contract

RTSPINE-008A is a docs/spec gate, not an implementation gate. It is recorded in
`docs/gates/RTSPINE_008A_PACKAGE_RESOURCE_PRESSURE_CONTRACT.md` and must be
treated as the source of truth for later RTSPINE-008 Package, File/VFS,
Resource, and RuntimeAsset packaged-validation write lanes.

The accepted pressure vocabulary is:

- `fixture cap`: a deterministic test/storage limit used by current unit tests;
- `budget assumption`: an explicit byte, table, dependency, or window budget
  selected by a gate;
- `pressure example`: a shipped-content-scale fact used to choose budgets, not
  a pass/fail target by itself;
- `runtime cap`: a hard runtime rejection boundary with an explicit status;
- `evidence threshold`: the focused command or test row proving one contract.

RTSPINE-008A chooses 64-bit archive byte ranges for future package/resource
pressure contracts. Any narrower fixture value must either be named as a
fixture-only cap or be validated before it reaches Package, File/VFS, Resource,
or RuntimeAsset state mutation.

The minimum hash coverage for follow-up gates is entry payload hash, entry
metadata hash, dependency table hash, package table hash, and optional archive
hash when a later gate names an archive container. RuntimeAsset packaged
validation must not claim pressure coverage until those hashes are checked
before Resource, Asset, RenderScene, RenderCore, RHI, Audio, or World outputs
are published.

The released Package/File/Resource write lanes are RTSPINE-008B Package
byte-range/index, RTSPINE-008C Package artifact hash/dependency integrity,
RTSPINE-008D File/VFS ranged IO, and RTSPINE-008E Resource payload
window/reference budget.
For RTSPINE-008B, `archive_byte_offset` and `archive_byte_size` are the
authoritative shipped-content pressure byte range. The existing `byte_offset`
and `byte_size` fields are legacy mirrors only and cannot be counted as
pressure evidence. RTSPINE-008C adds artifact payload, metadata, dependency
table, and package table hash validation before publish/mutation. RTSPINE-008D
adds File/VFS ranged read request/status, output-window, snapshot no-mutation,
and async small-output no-partial-copy evidence. RTSPINE-008E adds Resource
cache/decoded payload window metadata, reference-budget rejection, failed-window
no-mutation, and no Resource reference/residency mutation evidence. RTSPINE-008G
now consumes those lower Package/File/Resource gates for the RuntimeAsset
packaged validation bridge only. RTSPINE-008H adds RuntimeAsset transaction
rollback/proof for graph-load commit failures; broader Resource/File/VFS
follow-through still requires its own gate.

## 1.3 Continuous Multi-Agent Execution Governance

The coordinator must keep driving accepted L0/L1 work until the stop condition
is met. Do not stop after finishing one task. Do not wait for the human lead to
say "continue" while the active plan still has open work.

The architect should stay on architecture, dependency control, task design,
evidence governance, and rerouting. Routine implementation should go to
specialist agents whenever a clear owner exists, because the architect's context
budget is a project-level asset.

Every shared task must include:

- AI ETA;
- exact owner and role fit;
- exact scope and file or module surface;
- explicit non-goals;
- expected evidence;
- stale-owner timeout and reroute rule.

True parallelism means independent work surfaces. The team should run read-only
design audits, code-surface scouting, pressure gates, and disjoint implementation
lanes in parallel. The team must not split one serial dependency into duplicate
QA or full-test lanes that do not reduce calendar time.

Current immediate parallel pattern:

```text
RTSPINE-005 + RTSPINE-008C evidence docs/VQ closure
+ RTSPINE-006 RuntimeAsset invalid-target failure model evidence docs/VQ closure
+ RTSPINE-008D File/VFS ranged IO evidence docs/VQ closure
+ RTSPINE-008E Resource payload window evidence docs/VQ closure
+ RTSPINE-007 RuntimeAsset runtime instance mapping evidence docs sync
+ RTSPINE-008G RuntimeAsset packaged validation bridge evidence docs sync
+ RTSPINE-008H RuntimeAsset transaction rollback/proof evidence docs sync
```

RTSPINE-003 VQ accepted the target identity evidence gate by workspace task
`fdd78da4-da12-4956-b6ac-63ff9e377121`. RTSPINE-004 implementation and focused
QA are PASS at `ebe9ea35f531aa40133262b701e5e751f8ed9ccf`. RTSPINE-005
implementation and focused QA are PASS at `2bfe7e37d36ca711dd706728f21b1e4caecfd3db`
/ `d18f1679ebd389ecec506055764602591f5b9ab6`. RTSPINE-006 implementation and
focused QA are PASS at `96e0c024435f670c39ced019ff825b819a6830a3`. RTSPINE-007
implementation and focused QA are PASS at `37a112549190ac2123abcd72b5c688cdfa5b01e5`.
RTSPINE-008G RuntimeAsset packaged validation bridge implementation and focused
QA are PASS at `175b6542cf8460b279d1de8a5499e2cbd508c80a`. WorldObject-facing
mapping remains blocked until its own evidence gate is released. RTSPINE-008H
RuntimeAsset transaction rollback/proof implementation and focused QA are PASS
at `1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1`; broader Resource/File/VFS
follow-through remains blocked until its own evidence gate is released.


## 2. Current Progress Assessment

### 2.1 What Is Solid

The current mainline has meaningful first-slice coverage in the lower engine:

- Platform window/native surface/event pump.
- RHI backend-neutral boundary and D3D11 clear/present/capture path.
- D3D11 resource/pipeline primitives, visible triangle, static mesh, texture
  sampling, and swapchain resize contract.
- Audio test backend/mixer, PCM packet, PCM stream queue, XAudio2 callback
  first slice, and Resource-to-Audio packet import bridge.
- Input replay/action snapshot, Win32 bridge, and XInput bridge.
- File/VFS, package/load-plan, async file-completion, package/resource staging,
  Resource upload queue, upload completion commit, residency, cache payload,
  decode plan/result/decoded payload ownership.
- RenderCore fixture pass, material binding, submission batch, frame packet,
  graph skeleton, graph execution plan, shader/camera/draw/view-packet
  contracts.
- Object, Serialize, Script, World, and many World bridge slices exist as
  first-slice contracts.
- Fast gate and hardware-smoke coverage have grown substantially.

This is real progress. It should not be rolled back wholesale.

### 2.2 What Is Not Yet Solid

The current state is not yet a complete hardware layer or runtime engine:

- Many modules are still fixture/value/bridge proof, not production runtime
  behavior.
- Hardware smoke still contains environment-dependent skipped paths.
- The sample/evidence path is being cleaned from `Temp/AssetSmokeDemo` into
  more appropriate sample/tool/third-party locations; that cleanup must finish
  before the sample is treated as commercial evidence.
- L1/World bridge work has advanced faster than the L0 closure and can create
  dependency sprawl if it continues unchecked.
- `Bridge` classes are numerous and must be audited so they stay narrow
  adapters instead of hidden owners of policy or lifecycle.
- There is not yet a single engine-owned vertical sample that proves:

```text
window -> input -> resource read/decode -> upload -> render -> audio -> resize -> shutdown
```

### 2.3 Immediate Progress Adjustment

The landing team should make these changes now:

1. Freeze new broad feature gates for a short window.
2. Produce and maintain an L0 completion matrix.
3. Finish repository hygiene for sample/assets/third-party binaries.
4. Convert lower-engine proof into one engine sample.
5. Pause new World/Game Adapter/UI/gameplay expansion until L0 closure gates
   pass.
6. Reframe L1 work from "more bridges" to "runtime core vertical closure".

### 2.4 Runtime Visual Closure Correction

L0/L1 closure must not be reduced to fixture tests, value-contract rows, or a
single isolated sample screenshot. A pure runtime visual requirement does not
need an editor and does not need input.

The current `YuAssetSmokeDemo` route proves useful lower-layer facts, but it is
not enough to close the L1 runtime scene/render capability. L1 closure requires
a YuEngine-owned runtime visual sample that composes the lower layers into a
scene frame sequence:

```text
runtime app/session
-> resource/model/texture/material resolution
-> scene/object transform records
-> shared material with multiple texture inputs
-> shader/pipeline binding
-> RenderScene multi-entity submission
-> RenderCore/RHI multi-draw frame
-> camera tween sampling/update
-> bounded capture set written by runtime
```

The minimum visual sample is:

- one cube, one cylinder, and one cone;
- deterministic pseudo-random placement from a fixed seed;
- per-object rotation driven by runtime frame time;
- one material record applied to all three objects;
- at least three distinct texture inputs bound by that material record;
- explicit camera tween keyframes that move the perspective camera around the
  object group as a bounded screenshot/frame set;
- deterministic status/diagnostics that name the exact missing layer if the
  sample cannot run.

This is runtime scope. It must not be blocked on editor work, UI work, or input
handling. If this cannot pass after the claimed L0/L1 work, then L0/L1 is not
closed.

The implementation order must follow
`docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md`. Do not jump straight to the
final cube/cylinder/cone scene. Camera capture, scene placement,
geometry/model, material texture slots, shader/pipeline binding, animation
interpolation, transform application, and capture diagnostics are separate
foundation floors. Each floor must pass or name its blocker before the combined
runtime visual sample can be used as closure evidence.

RuntimeAsset shader/reflection hardening evidence covers NativeHlsl unsupported
backend prevalidation, `input=layout:none` as `InvalidInputLayout` before RHI
mutation, and non-canonical `position,texcoord` input with `textures=0`
success. QA evidence is focused-first: shader/reflection discovery reports `12`
rows, the required exact focused rows are `5/5` PASS, the focused
`YuRuntimeAssetDataClosedLoopTests` build passed, and diff/check/scans were
clean without running broad full CTest for that docs lane.

Current RuntimeAsset scene-animation evidence at
`f211f7f95299388987ccef00b4d1e8ee6f7bf0c1` covers bounded multi-clip animation
records, `selected_animation_clip_id`, explicit selected-clip sampling, reusable
runtime animation tables, RenderScene submission resampling, and missing selected
clip `InvalidDependency` without output mutation. QA evidence is focused-first:
the focused `YuRuntimeAssetDataClosedLoopTests` build passed, exact
scene-animation/runtime animation CTest discovery found `11` rows, exact
execution reports `11/11` PASS, `git diff --check` passed, added-line hygiene
passed, and dependency boundary checks passed without running broad full CTest
for the docs lane.

Current RuntimeAsset target identity evidence at
`5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4` covers
`RuntimeAssetTargetIdentityRecord` tables for scene node, model node, and
skeleton joint identities, with `target_id`, `parent_target_id`,
`scene_entity_id`, and `ordinal` values written to caller-provided output
buffers. QA evidence is focused-first: the focused
`YuRuntimeAssetDataClosedLoopTests` build passed, exact target identity plus
scene/runtime animation regression CTest discovery found `10` rows, exact
execution reports `10/10` PASS, `git diff --check` passed, added-line hygiene
passed, and production boundary scans passed without running broad full CTest
for the docs lane.

Current RuntimeAsset animation track target binding evidence at
`ebe9ea35f531aa40133262b701e5e751f8ed9ccf` covers SceneNode `target_id` plus
property binding through `RuntimeAssetAnimationTrackTargetBindingRecord`
caller-owned output. Focused QA task `2e2d5a4e-0bb0-4cf4-bd1b-ab3a87987b7f`
reports focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact discovery
of `17` rows, execution `17/17` PASS, `git diff --check` PASS, added-line
hygiene PASS, and boundary/non-goal scans PASS without running broad full CTest.
The RTSPINE-004 PASS does not open ModelNode/SkeletonJoint animation binding,
RTSPINE-007 runtime instance mapping, or any Package/Resource write lane.

Current RuntimeAsset minimal interpolation evidence at
`2bfe7e37d36ca711dd706728f21b1e4caecfd3db` covers Step/Linear fixed-time
sampling and no-mutation failures for unsupported interpolation and sample output
capacity. Focused QA task `951a3da8-6b13-4268-960e-407f65c40db7` reports focused
`YuRuntimeAssetDataClosedLoopTests` build PASS, exact RTSPINE-005 interpolation
discovery `3`, execution `3/3` PASS, non-Package RuntimeAsset animation
whitelist discovery/execution `23/23` PASS, `git diff --check` PASS, added-line
hygiene PASS, and production boundary/non-goal scans PASS without running
broad/full CTest. The PASS does not open ModelNode/SkeletonJoint animation
binding or RTSPINE-007 runtime instance mapping.

Current RuntimeAsset invalid-target failure model evidence at
`96e0c024435f670c39ced019ff825b819a6830a3` covers target-family mismatch
rejection and sample failure diagnostics without output mutation. Focused QA
task `6d02c260-936a-456b-917b-5c2802bbb666` reports an isolated clean worktree
at `96e0c024`, focused `YuRuntimeAssetDataClosedLoopTests` build PASS, focused
RuntimeAsset regex discovery/execution `8/8` PASS, exact new RTSPINE-006 rows
`2/2` PASS, `git diff --check` PASS, added-line hygiene PASS, non-goal boundary
scan PASS, and no broad/full CTest. The PASS does not open RTSPINE-007 runtime
instance mapping, RTSPINE-008H, or the RuntimeAsset packaged validation bridge
by itself; the separate RTSPINE-008G gate below opens that bridge.

Current RuntimeAsset runtime instance mapping evidence at
`37a112549190ac2123abcd72b5c688cdfa5b01e5` covers caller-owned runtime
instance mapping rows for SceneNode targets, animation target binding through
the mapping, missing scene entity failure without mutation, output capacity
failure without mutation, and unsupported ModelNode/SkeletonJoint target-family
mapping failures without mutation. Focused QA task
`6b6baf5f-2381-4b9c-89b1-4411fba53d23` reports focused
`YuRuntimeAssetDataClosedLoopTests` build PASS,
`RuntimeAssetData_(RuntimeInstanceMapping|AnimationTrackTargetBinding|AnimationFailureModel|RuntimeAnimationTables)`
discovery/execution `12/12` PASS, exact RuntimeInstanceMapping rows `5/5`
PASS, commit-level `git diff --check` PASS, added-line hygiene PASS, boundary
scan PASS, read-only QA with a clean final repo, and no broad/full CTest. The
PASS does not open direct WorldObject/editor mapping, RTSPINE-008H, or the
RuntimeAsset packaged validation bridge by itself; the separate RTSPINE-008G gate
below opens that bridge.

Current Package artifact hash/dependency evidence at
`d18f1679ebd389ecec506055764602591f5b9ab6` covers RTSPINE-008C Package-only
payload, metadata, dependency table, and package table hash validation.
Focused QA task `ba135e38-b73e-4294-b449-97a04b33b982` reports
`YuPackageTests` build PASS, `^Package_` discovery/execution `35/35` PASS,
the two new integrity rows `2/2` PASS, `git diff --check` PASS, added-line
hygiene PASS, and boundary scans PASS without running broad/full CTest. The
008C PASS did not itself open File/VFS ranged IO; the separate 008D gate below
does. It did not itself open Resource payload windows; the separate 008E gate
below does. It did not itself open the RuntimeAsset packaged validation bridge;
the separate RTSPINE-008G gate below does. It does not open RTSPINE-008H.

Current File/VFS ranged IO evidence at
`c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e` covers RTSPINE-008D ranged
`FileReadRequest`, `InvalidRange` and `RangeOutOfBounds` status reporting,
snapshot no-mutation on failed ranges, and async ranged output-too-small
no-partial-copy behavior. Focused QA task
`aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports `YuFileTests` build PASS,
`^File_` discovery/execution `23/23` PASS, ranged subset `4/4` PASS,
`git diff --check` PASS, added-line hygiene PASS, and boundary scans PASS
without running broad/full CTest. The PASS does not open Resource payload
windows by itself; the separate 008E gate below does. It does not open
RuntimeAsset packaged validation bridge by itself; the separate RTSPINE-008G gate
below does. It does not open RTSPINE-008H.

Current Resource payload window/reference budget evidence at
`8bb8eff9c98d2a0aa5050c5da6ad94049fa894be` covers RTSPINE-008E cache payload
window metadata, decoded payload window metadata, reference-budget failures,
window overflow/mismatch no-mutation failures, and the proof that payload window
reads do not change Resource reference count or residency. Focused QA task
`b4fa51c3-aefc-4714-b5d8-062f8a933ac9` reports `YuResourceTests` build PASS,
Resource window/reference discovery exactly `7` rows, execution `7/7` PASS,
commit-level `git diff --check` PASS, added-line hygiene PASS, non-goal boundary
scan PASS, and no broad/full CTest. The PASS does not open RuntimeAsset packaged
validation bridge by itself; the separate RTSPINE-008G gate below does. It does
not open RTSPINE-008H.

Current Package dependency closure and budgeted load plan evidence at
`8509f7e1b6ba15e79c574357a465ddfff4d80e10` covers transitive dependency
closure before the root record, shared dependency de-duplication with declaration
order, load-plan record budget failure without mutation, and archive byte budget
failure without mutation. Focused QA task
`4f199c8e-99a4-43b4-a776-8960285ffdaf` reports allowed Package/CMake scope,
`YuPackageTests` build PASS, exact 008F rows `4/4` PASS, `^Package_` focused
suite `39/39` PASS, `git diff --check` PASS, added-line hygiene/scope scan
PASS, no broad/full CTest, and no QA edits/staging/commits. The PASS does not
open RuntimeAsset packaged validation bridge by itself; the separate
RTSPINE-008G gate below does. It does not open RTSPINE-008H.

Current RuntimeAsset packaged validation bridge evidence at
`175b6542cf8460b279d1de8a5499e2cbd508c80a` covers archive byte-range and payload
hash preflight before graph-load mutation, dependency/load-plan record
validation, duplicate load-plan record rejection without mutation, PackageStatus
failure mapping, and ProductRun packaged validation failure reporting without
graph mutation. Focused QA task `35fdc7a2-c09d-416a-95aa-b4aabdb05d0f` reports
focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact RTSPINE-008G rows
`5/5` PASS, adjacent packaged/product rows `8/8` PASS, committed scope exactly
`CMakeLists.txt`, `RuntimeAssetData.h/.cpp`, and
`RuntimeAssetDataClosedLoopTests.cpp`, `git diff --check` PASS, read-only QA with
a clean repo, and no broad/full CTest. The 008G PASS opens only the RuntimeAsset
packaged validation bridge; the separate 008H gate below covers transaction
rollback/proof.

RTSPINE-008H RuntimeAsset transaction rollback/proof is PASS at
`1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1`. It adds a graph-load rollback
journal, rollback status/proof fields, and `SetCommitFailureAndRollback` so
commit failures roll back previously committed RuntimeAsset records and restore
Resource/Asset snapshots without output mutation. Focused QA task
`1ec65e79-70f2-4fe5-8f08-6fb0ba2371fd` reports focused
`YuRuntimeAssetDataClosedLoopTests` build PASS, exact
`RuntimeAssetData_LoaderCommitFailureRollsBackCommittedRecords` row `1/1` PASS,
focused rollback/commit/adjacent packaged/product set `19/19` PASS, committed
scope exactly `CMakeLists.txt`, `RuntimeAssetData.h/.cpp`, and
`RuntimeAssetDataClosedLoopTests.cpp`, `git diff --check` PASS, added-line
hygiene PASS, no Package/File/Resource lower-module changes, no docs/VQ changes,
and no broad/full CTest. The PASS does not open WorldObject/editor mapping,
broader Resource/File/VFS follow-through, RenderScene/RHI production expansion,
or unrelated animation mapping.

## 3. Updated Layer Model

The old restart plan used a broader L0-L7 diagram. For the next execution
window, use this simplified delivery model.

### L0: Platform / Hardware-Facing Lower Layer

L0 owns the stable lower-engine abilities that touch OS, device, IO,
backend-specific systems, and frame submission.

Included modules:

- `YuPlatform`
- `YuRhi`
- D3D11 backend internals
- `YuAudio`
- `YuInput`
- `YuFile`
- `YuThread`
- `YuMemory`
- `YuDiagnostics`
- lower `YuPackage` load-plan primitives
- lower `YuResource` / `YuStreaming` upload/decode/cache primitives
- lower `YuRenderCore` frame/draw/material/view/graph execution primitives
- `YuHardware` / FrameHost integration shell

L0 must not own:

- gameplay rules
- scene policy
- UI business
- old game package compatibility
- project-specific BGM/SE IDs
- TouhouNewWorld title/save/new-game logic
- editor ecosystem
- public plugin contracts

L0 exit condition:

```text
An engine-owned sample can run on the target Windows/D3D11/XAudio2/XInput path,
or explicitly skip unavailable hardware with graded reasons, while Debug,
Release, fast gate, hardware smoke, and sample smoke remain reproducible.
```

### L1: Runtime Core / Engine Framework Layer

L1 owns the project-independent runtime framework that a real game project uses
to create, update, save, load, render, and debug a scene.

Included modules and future modules:

- `YuKernel` runtime services and game loop integration
- `YuObject`
- `YuSerialize`
- `YuScript` native bridge
- `YuWorld`
- Scene lifecycle and scene assembly
- Transform/object/component model
- Asset Manager over Resource/Package/File
- Render Scene over RenderCore/RHI
- Audio Scene over Audio/Resource
- Input Mapping over Input devices/actions
- Camera and capture records as runtime values
- Animation clip/track interpolation and transform application
- Save/Profile/Config primitives
- Runtime diagnostics surface

L1 must not own:

- D3D11/XAudio2/Win32 native details
- project-specific title flow
- old package compatibility
- editor plugin ecosystem
- broad public scripting ecosystem
- gameplay-specific object types unless promoted by repeated use

L1 exit condition:

```text
A small project scene can load, instantiate objects/components, bind resources,
respond to input, submit render/audio through L0, save/reload core state, and
shut down cleanly without project-specific shortcuts.
```

Runtime visual addendum:

```text
A project-independent visual sample can render and capture a deterministic
multi-object scene through the L1 scene/render/material/resource path. The
minimum accepted sample is the cube/cylinder/cone orbit-camera capture set from
section 2.4, built from the module floors in
`docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md`.
```

### L2: Project / Product Layer

L2 is not part of this document's implementation plan, but its boundary matters.

L2 owns:

- game-specific actors, enemies, stages, combat, UI screens, save semantics,
  project schema, project data, project-specific tools

L2 must not backflow into L0 or L1. If a project need exposes a missing engine
capability, the work returns to the owning L0/L1 gate.

### Tooling / QA Plane

Tooling is cross-cutting and not a runtime dependency layer.

It includes:

- asset cooker
- validator
- sample runner
- packager
- profiler
- debug overlay
- frame/audio/resource capture tools
- CI/test presets

Tooling may observe runtime state through explicit diagnostic/capture
interfaces. It must not own runtime behavior.

## 4. Non-Goals For The Next Window

Do not build these now:

- Public developer ecosystem.
- Marketplace or plugin store.
- Public stable plugin ABI.
- General editor extension platform.
- UE/Unity-compatible scripting ecosystem.
- Multi-backend rendering abstraction beyond the current target closure.
- Old TouhouNewWorld package/runtime compatibility layer.
- Generic ECS/DOTS/reflection universe.
- UI business or title menu.
- Gameplay systems.
- Network/cloud/account/store systems.
- Full editor.

Do build narrow internal production tools where they directly unblock the
engine sample, asset validation, profiling, or packaging.

## 5. L0 Execution Plan

### L0-0: Completion Matrix And Freeze

Duration: 1 to 2 days

Owner split:

- Architect: definitions and acceptance language
- PM/gate owner: state matrix and sequencing
- QA/code review: current risks and verification commands
- Performance owner: hot-path and allocation expectations
- Evidence owner: sample/third-party/licensing provenance

Deliverable:

`docs/YUENGINE_L0_COMPLETION_MATRIX.md`

Matrix columns:

```text
Subsystem
Current module/files
Current gate or commit evidence
State: Done / FirstSlice / BlockedByEnv / Deferred / Rework
Required closure evidence
Required test command
Allowed skips
Forbidden scope
Owner
Next action
```

Minimum subsystem rows:

- Platform window/event/native surface
- RHI backend-neutral contract
- D3D11 device/swapchain/present/capture
- D3D11 resource/pipeline primitives
- RHI visible triangle/static mesh/texture sampling
- RHI resize
- RenderCore frame packet/view packet/draw/material/graph execution
- File/VFS loose read/write and path policy
- Package load-plan/staging
- Thread worker / async file-completion
- Resource cache/decode/upload/commit/residency
- Audio mixer/test backend
- XAudio2 callback
- Audio PCM packet/stream queue
- Audio Resource import bridge
- Input replay/action
- Win32 input bridge
- XInput bridge
- Diagnostics/memory/thread cost surfaces
- HardwareFrameHost
- Engine sample
- Third-party/sample asset hygiene

Acceptance:

- Matrix is committed.
- Every row has a state and next action.
- Skipped hardware tests are graded, not buried in PASS language.
- No row claims "complete" without a reproducible command and failure model.

Stop conditions:

- Any row cannot name its owner.
- Any bridge row owns lifecycle for both sides.
- Any sample depends on isolated binaries not built through YuEngine.

### L0-1: Repository And Sample Hygiene

Duration: 1 to 2 days, can overlap with matrix

Goal:

Remove the current evidence weakness around temporary sample assets and
third-party binaries.

Tasks:

1. Move engine sample source under `Samples/`.
2. Move helper tools under `Tools/`.
3. Move third-party runtime binaries or source declarations under `ThirdParty/`
   with license/provenance notes.
4. Ensure generated sample binaries and capture outputs are ignored.
5. Ensure sample assets are either source-controlled intentional fixtures or
   generated by a documented tool.
6. Add sample README explaining:

```text
what is built
what uses YuEngine build outputs
what third-party artifacts are used
what command runs the sample
what result proves
what result does not prove
```

Acceptance:

- No committed executable/dll/capture output under `Temp/`.
- `git status` and `git diff --check` are clean after intended commits.
- Sample can be built or intentionally skipped by a documented command.
- License/provenance for third-party audio/image/codec artifacts is explicit.

### L0-2: Platform And Frame Pump Closure

Duration: 2 to 3 days

Goal:

Make the window/event/native-surface boundary stable enough for RHI and
FrameHost.

Tasks:

1. Confirm Win32 window lifecycle:
   - create
   - show/hide if required
   - resize
   - focus
   - close
   - destroy
2. Confirm event pump behavior:
   - bounded event queue
   - deterministic drain
   - no hidden sleeping in tests
   - close event maps to engine state
3. Confirm native surface descriptor:
   - opaque public value
   - no public HWND/HINSTANCE leak unless explicitly gated
   - D3D11 backend consumes through private adapter path
4. Confirm error behavior:
   - invalid descriptors rejected
   - double destroy is safe or explicit failure
   - unsupported headless/window mode is explicit

Acceptance tests:

- Platform create/destroy fixture
- resize/focus/close event ordering fixture
- native surface descriptor validation fixture
- FrameHost can consume Platform events without owning Platform internals

Forbidden:

- UI routing
- gameplay input
- Resource/RenderCore ownership
- public backend-native type leaks

### L0-3: RHI And D3D11 Closure

Duration: 4 to 7 days

Goal:

Complete one stable D3D11-backed path through the backend-neutral RHI boundary.

Tasks:

1. Backend-neutral device contract:
   - capability flags
   - swapchain support state
   - error statuses
   - resource handle generation
2. D3D11 device/swapchain:
   - create
   - clear
   - present
   - capture bytes
   - resize
   - shutdown
3. Resource primitives:
   - buffer
   - texture
   - sampler
   - shader
   - input layout
   - pipeline
   - fence/retirement ledger
4. Draw proof:
   - visible triangle
   - indexed static mesh
   - texture sampling
5. Resize proof:
   - backbuffer generation changes
   - stale color target rejected
   - RenderCore receives explicit resize state later, not in this gate
6. Failure proof:
   - unsupported device
   - invalid descriptor
   - stale handle
   - resource creation failure where possible

Acceptance tests:

- existing `windows-fast-gate` remains deterministic
- `windows-hardware-smoke` D3D11 rows pass on a supported machine
- capture-byte assertions prove behavior
- no screenshot/manual proof requirement

Forbidden:

- scene traversal
- material graph
- shader compiler/toolchain expansion
- World/UI/Game Adapter
- Resource owning backend-native objects
- public D3D11 header leakage

### L0-4: RenderCore Lower Closure

Duration: 3 to 5 days

Goal:

Make RenderCore a stable frame-assembly layer over RHI without becoming scene
or gameplay.

Tasks:

1. View packet:
   - camera constants
   - viewport/scissor
   - frame index
   - resize awareness as value input only
2. Draw packet:
   - geometry handles
   - material binding values
   - shader program values
   - deterministic ordering rules
3. Frame pipeline:
   - prepare
   - validate
   - submit one or more bounded batches
   - present through RHI-owned swapchain path
4. Graph skeleton/execution:
   - declared passes
   - dependencies
   - no transient aliasing yet
   - no command-list parallelism yet
5. Failure:
   - missing resource
   - stale handle
   - graph validation failure
   - unsupported backend capability

Acceptance:

- RenderCore sample path submits through RHI only.
- RenderCore does not include D3D11 headers.
- RenderCore does not depend on World, UI, Script, or Game Adapter.
- Frame packet tests prove no hidden growth in hot fixture paths.

Forbidden:

- scene renderer
- camera components
- material graph editor
- shader compiler pipeline
- UI rendering

### L0-5: File, Package, Resource, Streaming Closure

Duration: 5 to 8 days

Goal:

Create a real lower asset path for the L0 sample without building a full asset
pipeline.

Tasks:

1. File/VFS:
   - loose read
   - loose write if needed by sample/tools
   - path normalization
   - mount ordering
   - small fixture bounds
2. Package:
   - manifest/load-plan values
   - staging queue
   - no original package parser expansion in L0 closure
3. Resource:
   - handle validation
   - cache payload
   - decode plan
   - decode result
   - decoded payload ownership
   - residency budget
4. Streaming:
   - upload queue
   - upload completion commit
   - deterministic counters
5. Bridges:
   - Resource decoded texture -> RHI upload request
   - Resource decoded PCM -> Audio packet request
6. Sample path:
   - at least one texture or mesh fixture travels through File/Resource/Upload
   - at least one PCM packet or stream queue fixture travels through Audio path

Acceptance:

- One sample asset path uses YuEngine modules rather than a standalone demo.
- Resource core does not own RHI device lifecycle.
- Audio core does not parse Resource payloads directly.
- File/Package does not know about World or project gameplay.

Forbidden:

- old game package compatibility
- full cooker
- hot reload
- scene loader
- game-specific resource IDs
- report-shaped APIs

### L0-6: Audio Closure

Duration: 3 to 5 days

Goal:

Make audio hardware/backend proof honest and repeatable.

Tasks:

1. Test backend/mixer remains deterministic.
2. PCM packet records are stable.
3. PCM stream queue drains into caller-owned output.
4. XAudio2 callback path:
   - initializes on supported machines
   - gives explicit unavailable status on unsupported machines
   - does not silently pass when device is missing
   - avoids allocation/blocking in callback path
5. Resource-to-Audio bridge:
   - maps metadata only
   - does not move decoded byte ownership into Audio bridge

Acceptance:

- Fast tests prove deterministic mixer/packet/queue behavior.
- Hardware smoke has at least one supported-machine XAudio2 PASS path, or a
  documented environment skip row in the L0 matrix.
- Callback path has no file IO, locks, sleeps, or unbounded allocation.

Forbidden:

- BGM/SE business IDs
- audio scene
- UI volume menu
- gameplay audio events
- codec ecosystem beyond sample need

### L0-7: Input Closure

Duration: 2 to 4 days

Goal:

Make input device events stable before L1 action/gameplay mapping expands.

Tasks:

1. Win32 keyboard/mouse bridge:
   - deterministic event translation
   - focus/close/resize separation
   - bounded queue behavior
2. XInput bridge:
   - available/unavailable
   - connect/disconnect
   - stable gamepad index behavior
   - no silent success on missing hardware
3. Input core:
   - replayable snapshots
   - action values
   - no platform-native leak

Acceptance:

- Fast tests prove replay/action behavior.
- Hardware smoke proves or explicitly skips XInput based on environment.
- L1 receives value snapshots only.

Forbidden:

- UI navigation
- text input framework
- gameplay command mapping
- World or Script callbacks

### L0-8: Diagnostics, Memory, Thread Cost Closure

Duration: 2 to 4 days

Goal:

Ensure the L0 sample and lower-engine paths can be debugged without reports or
diagnostics owning runtime behavior.

Tasks:

1. Memory:
   - owner/tag vocabulary
   - allocation counters where available
   - limitation notes for untracked CRT/STL/general heap paths
2. Thread:
   - worker lifecycle
   - async completion queue
   - deterministic shutdown
   - no sleep-based proof
3. Diagnostics:
   - bounded events/counters
   - disabled behavior neutrality
   - no report-driven core API
4. Performance smoke:
   - hot fixture paths do not grow containers unexpectedly
   - sample path has basic frame/resource/audio counters

Acceptance:

- Diagnostics can be disabled without changing behavior.
- Memory/thread/diagnostic tests stay in fast gate.
- Sample emits bounded optional diagnostics only.

Forbidden:

- JSON reports as runtime API
- unbounded logging on hot paths
- report/capture/oracle ownership inside L0 modules

### L0-9: HardwareFrameHost And Engine Sample

Duration: 4 to 7 days

Goal:

Prove L0 as an engine-owned vertical path.

Sample flow:

```text
Start process
Create platform window
Create native surface descriptor
Create RHI device/swapchain
Load small fixture asset through File/Resource path
Upload texture or mesh resource through Streaming/RHI path
Prepare RenderCore view/draw/material packet
Poll Input snapshot
Submit frame
Present
Optionally play PCM stream
Handle resize
Shutdown cleanly
```

Acceptance commands:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-hardware-smoke --output-on-failure
cmake --preset windows-release
cmake --build --preset windows-release
ctest --test-dir build/windows-release-vs -C Release --output-on-failure
```

The exact release build directory should follow the active preset output path
used by the repository.

Acceptance:

- Sample is built from YuEngine source tree.
- Sample does not depend on committed generated binaries.
- Sample proves a real module path, not an isolated demo.
- Hardware unavailable cases are explicit skip/fail states.
- No project gameplay is introduced.

## 6. L1 Execution Plan

L1 should begin only after the L0 matrix is accepted and the L0 vertical sample
path is either complete or has documented blockers that do not affect L1 design.

### L1-0: Runtime Core Freeze

Duration: 1 to 2 days

Deliverable:

`docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`

Rows:

- Kernel/game loop
- Frame context/time
- Object registry
- Component identity
- Transform records
- World lifecycle
- Scene assembly
- Asset manager
- Render scene
- Audio scene
- Input mapping
- Script native bridge
- Serialization/save
- Config/profile
- Runtime diagnostics

Each row states:

```text
Owner
Depends on L0 modules
Allowed L1 dependencies
Forbidden dependencies
Current first-slice status
Required vertical-slice evidence
Performance risk
Test route
```

### L1-1: Runtime Kernel And Game Loop

Duration: 3 to 5 days

Goal:

Provide a runtime loop that coordinates L0 without becoming gameplay.

Tasks:

1. Define `RuntimeApp` or equivalent host over existing Kernel/FrameHost.
2. Define frame phases:

```text
BeginFrame
PollPlatform
PollInput
LoadOrCommitResources
UpdateWorld
PrepareRender
SubmitAudio
SubmitRender
Present
EndFrame
```

3. Define `FrameContext`:
   - frame index
   - delta time
   - fixed/update mode
   - input snapshot reference
   - diagnostics sink reference
4. Define pause/shutdown/error states.
5. Keep service registry stable and avoid global mutable business state.

Acceptance:

- Runtime loop can run zero-world frames.
- Runtime loop can run a small fixed-count headless test.
- Shutdown order is deterministic.
- L0 failure statuses propagate without throwing project-specific behavior.

Forbidden:

- gameplay update policy
- scene file loading
- UI/title behavior
- old runtime service state

### L1-2: Object, Component, Transform Baseline

Duration: 5 to 8 days

Goal:

Turn current `YuObject` and World sidecars into a coherent runtime identity
model without overbuilding ECS.

Tasks:

1. Object identity:
   - generation-checked handles
   - lifetime states
   - acquire/release validation
2. Component identity:
   - component type IDs
   - component slot IDs
   - attachment records
   - query bridge stays read-only
3. Transform:
   - POD transform record
   - local/world flag policy deferred unless needed
   - no hierarchy until a separate gate
4. Ownership:
   - World owns instance membership
   - Object owns object lifetime
   - Component bridge owns attachment sidecar
   - Transform bridge owns transform sidecar
5. No reflection by default.

Acceptance:

- Create/destroy object fixture.
- Attach/query component fixture.
- Set/query transform fixture.
- Destroy object invalidates relevant sidecar bindings through explicit caller
  path or documented future gate.
- No hidden growth in measured hot update/query path.

Forbidden:

- full ECS
- actor behavior lifecycle
- transform hierarchy
- reflection system
- script callbacks
- render/audio component behavior

### L1-3: World And Scene Assembly Core

Duration: 6 to 10 days

Goal:

Move from bridge fragments to a scene assembly model that remains caller-owned
and explicit.

Tasks:

1. World lifecycle:
   - create
   - update phases
   - destroy
2. Scene assembly records:
   - object identity record
   - transform record
   - component attachment record
   - component-resource binding record
3. Restore plan:
   - no-mutation preflight
   - duplicate detection
   - missing object/resource detection
   - deterministic status report
4. Active restore coordinator:
   - separate gate after proof
   - validate all before mutation
   - rollback strategy explicit before implementation
5. Manifest stream:
   - stream transport stays separate from active restore

Acceptance:

- Load a synthetic scene record set into caller-owned scratch.
- Preflight produces deterministic plan.
- Active restore only happens after an approved gate and no-mutation proof.
- World core does not depend directly on Resource/File/Package internals.

Forbidden:

- scene loader from disk as first step
- save-game policy
- object construction by Serialize
- resource loading inside scene restore
- game adapter semantics

### L1-4: Asset Manager Over Resource

Duration: 5 to 8 days

Goal:

Provide the runtime-facing asset layer without turning Resource into gameplay.

Tasks:

1. Asset handle:
   - stable ID
   - type
   - generation
   - state
2. Asset state:
   - unloaded
   - loading
   - decoded
   - uploaded/resident
   - failed
3. Dependency records:
   - explicit dependency list
   - no recursive load explosion
   - bounded traversal
4. Residency:
   - pin/unpin policy
   - budget status
   - eviction candidate query
5. L0 bridge:
   - uses Resource cache/decode/upload primitives
   - does not own RHI/Audio device lifecycle

Acceptance:

- Request asset by manifest/load-plan value.
- Decode/import a fixture payload.
- Produce texture/audio ready records.
- Query failure state.
- Release asset and observe residency/counter state.

Forbidden:

- full hot reload
- external asset ecosystem
- editor import UX
- old package compatibility
- gameplay asset semantics

### L1-5: Render Scene

Duration: 6 to 10 days

Goal:

Create a minimal runtime render scene that feeds RenderCore without owning RHI.

Tasks:

1. Render entity records:
   - object ID
   - transform reference
   - mesh/geometry asset handle
   - material asset handle
2. Camera:
   - camera record
   - active camera selection
   - view/projection values
3. Visibility:
   - initial simple full-list traversal
   - culling deferred unless needed
4. Packet assembly:
   - render records -> draw packets
   - material binding values
   - view packet
5. Resize:
   - receive L0/RHI resize state as value
   - do not own Platform event pump

Acceptance:

- Synthetic scene with one camera and one mesh produces RenderCore frame packet.
- RenderScene has no D3D11 includes.
- RenderScene can run with Null RHI or test fixture where possible.
- Missing mesh/material produces explicit status, not crash.

Forbidden:

- material graph
- lighting system
- animation/skinning
- UI rendering
- editor viewport

### L1-6: Audio Scene

Duration: 4 to 7 days

Goal:

Provide runtime audio source/bus concepts over L0 Audio without BGM/SE business
IDs.

Tasks:

1. Sound asset handle binding.
2. Audio source record:
   - play
   - stop
   - paused
   - gain
   - loop flag if required
3. Bus/mixer routing:
   - minimal fixed bus IDs
   - no editor graph
4. Runtime update:
   - source state -> PCM stream queue request
   - unavailable backend status propagated

Acceptance:

- Synthetic source plays through test backend.
- Supported hardware path can submit through callback where available.
- Missing asset/backend produces explicit status.
- No blocking IO or allocation in callback path.

Forbidden:

- BGM/SE project service IDs
- UI volume menu
- full DSP graph
- audio editor

### L1-7: Input Mapping

Duration: 3 to 5 days

Goal:

Translate L0 device snapshots into project-independent runtime commands.

Tasks:

1. Action map:
   - action ID
   - device binding
   - button/axis value
2. Input context:
   - active context list
   - focus or mode value
3. Runtime command snapshot:
   - pressed/released/held
   - axis value
   - frame index
4. Replay:
   - deterministic replay from L0 snapshots

Acceptance:

- Keyboard action fixture.
- XInput action fixture with unavailable skip/status.
- Input mapping does not call World or Script directly.

Forbidden:

- gameplay command execution
- UI navigation policy
- text input framework

### L1-8: Serialization, Save, Config

Duration: 5 to 8 days

Goal:

Use `YuSerialize` as a value stream, not as object construction or save policy.

Tasks:

1. Project-independent config record.
2. Scene assembly record stream.
3. Object/component/transform snapshot stream.
4. Versioning:
   - stream version
   - record version
   - explicit unsupported status
5. Save boundary:
   - write caller-owned buffers
   - file persistence is a separate File/Tool path

Acceptance:

- Serialize/deserialize a synthetic scene assembly record set.
- Reject unsupported version without mutation.
- No File/Package/Resource dependency inside Serialize core.

Forbidden:

- save-game business policy
- original save compatibility
- object construction in Serialize
- reflection system

### L1-9: Script Native Bridge Runtime Use

Duration: 4 to 7 days

Goal:

Keep script as a narrow call boundary before any VM or old script evidence.

Tasks:

1. Stable call IDs.
2. Caller-provided argument/value slots.
3. Native registry.
4. Error statuses.
5. Runtime phase dispatch from World remains an adapter, not World core.

Acceptance:

- Register and call a synthetic native function.
- World phase dispatch bridge maps phase trace to call ID.
- Missing call returns explicit failure.
- No stringly dispatch in hot path.

Forbidden:

- bytecode VM
- reflection
- original-game service state
- script-owned World/Resource/File access

### L1-10: Runtime Diagnostics Surface

Duration: 3 to 5 days

Goal:

Expose runtime state enough for internal debugging without turning diagnostics
into behavior.

Tasks:

1. Frame timing counter.
2. Object/world/resource/audio/render counts.
3. Last error ring or bounded event channel.
4. Optional debug overlay hook later.
5. Diagnostics-disabled equivalence tests.

Acceptance:

- Runtime runs with diagnostics disabled.
- Runtime emits bounded counters when enabled.
- No log/report output is required for behavior correctness.

Forbidden:

- report JSON as core API
- unbounded logging
- oracle/capture controlling runtime flow

### L1-11: Minimal Runtime Vertical Sample

Duration: 7 to 14 days after L1 subsystems are ready

Goal:

Prove L1 with a small project-independent scene.

Sample flow:

```text
Boot runtime
Create world
Load synthetic scene manifest
Create object records
Attach transform/component records
Bind texture/audio resource handles
Map input to runtime command
Update world for fixed frames
Assemble render scene
Submit RenderCore/RHI frame
Submit AudioScene/Audio frame
Serialize snapshot
Destroy world
Shutdown runtime
```

Acceptance:

- Runs as a CTest or sample command.
- Does not use TouhouNewWorld business logic.
- Does not parse old package runtime.
- Has deterministic output/status.
- Has Debug and Release validation.

## 7. Bridge Audit Plan

Bridge count is high enough that it needs explicit control.

Deliverable:

`docs/YUENGINE_BRIDGE_AUDIT.md`

Required fields:

```text
Bridge name
Source module
Destination module
Direction
Data handed off
Who owns source lifecycle
Who owns destination lifecycle
Failure statuses
Capacity bounds
Tests
Allowed layer crossing
Risk: Low / Medium / High
Required action
```

High-risk bridge patterns:

- crosses more than two adjacent conceptual layers
- reads and mutates multiple owner states
- has cache or lifecycle policy
- includes backend-native headers
- includes project/game-specific types
- uses reports/logs as behavior transport
- exists only to make a test pass

Required actions:

- Low risk: document and keep.
- Medium risk: add tests or split adapter from policy.
- High risk: split, move, or block future dependency until rewritten.

## 8. Team Work Breakdown

Use parallelism, but keep integration serial.

### Architecture / PM Lane

Tasks:

- L0 completion matrix.
- L1 runtime matrix.
- bridge audit.
- gate names and sequencing.
- final acceptance language.

### Render/RHI Lane

Tasks:

- D3D11 closure.
- swapchain resize proof.
- RenderCore frame sample.
- render sample integration.

### Platform/Input Lane

Tasks:

- Win32 event pump stabilization.
- Input bridge/XInput environment proof.
- L1 input mapping proposal after L0.

### Audio Lane

Tasks:

- XAudio2 callback proof.
- PCM stream queue closure.
- Audio scene proposal after L0.

### File/Resource/Streaming Lane

Tasks:

- sample asset path.
- decoded payload to RHI/Audio bridges.
- residency/cache/decode closure.
- L1 asset manager proposal.

### Runtime Core Lane

Tasks:

- kernel/game loop.
- object/component/transform.
- world/scene assembly.
- serialization/config/save.

### QA/Performance Lane

Tasks:

- fast/hardware/release commands.
- allocation/hot-path smoke.
- sample smoke.
- dirty worktree/repo hygiene.

Effective parallelism:

```text
L0 closure: 6-7 effective people out of 10
L1 runtime closure: 5-6 effective people out of 10
Final sample integration: 2-3 people, mostly serial
```

Reason:

Render, audio, input, file/resource, and QA can run in parallel. FrameHost,
sample, runtime loop, and scene integration must serialize at the end to avoid
contradictory ownership decisions.

## 9. Verification Policy

At every handoff:

```text
git status --short --branch
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

For hardware closure:

```text
ctest --preset windows-hardware-smoke --output-on-failure
```

For release closure:

```text
cmake --preset windows-release
cmake --build --preset windows-release
ctest --test-dir <active release build dir> -C Release --output-on-failure
```

The exact release build dir must be read from the active preset or current repo
convention before execution.

Do not weaken fast gate to make hardware tests pass. Hardware tests are isolated
through labels/presets and explicit skip grading.

## 10. Done Definitions

### L0 Closure Candidate

L0 can be called a closure candidate when:

- L0 completion matrix is committed.
- Sample/repo hygiene is complete.
- Debug fast gate passes.
- Release tests pass.
- Hardware smoke has no unexpected failures.
- Environment skips are documented and graded.
- One YuEngine-owned sample exercises platform, RHI, input, resource, render,
  audio or explicit audio skip, resize, and shutdown.
- Bridge audit has no unresolved high-risk L0 bridge.

### L0 Done For Current Product

L0 can be called done for the current product when:

- Required hardware paths pass on the target production machine.
- Sample uses intended asset path.
- Failure states are recoverable or explicitly fatal.
- Hot-path budget smoke exists for render submit, audio callback, input poll,
  resource upload, diagnostics disabled/enabled, and thread/file completion.
- No temporary demo or evidence artifact is required for proof.

### L1 Closure Candidate

L1 can be called a closure candidate when:

- L1 matrix is committed.
- Runtime loop runs.
- Object/component/transform baseline is stable.
- Scene assembly preflight and controlled restore exist.
- Asset manager can bind runtime assets over L0.
- RenderScene and AudioScene can consume runtime records and submit to L0.
- Input mapping produces runtime command snapshots.
- Serialization/config can save and read a synthetic scene snapshot.
- Runtime diagnostics is bounded and optional.
- One project-independent vertical sample passes.

### Not Done

Do not call the engine commercially complete until:

- L2 project playable exists.
- Packaging/cook/release flow exists.
- Internal tools can validate assets and diagnose failures.
- Performance budgets are measured on realistic content.
- Regression tests cover startup, scene load, resource errors, device issues,
  and shutdown.

## 11. Required Future Documents

Create these documents in order:

1. `docs/YUENGINE_L0_COMPLETION_MATRIX.md`
2. `docs/YUENGINE_BRIDGE_AUDIT.md`
3. `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`
4. `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`
5. `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`
6. `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md`

Do not create new broad implementation tasks until document 1 and document 2
exist and are accepted.

## 12. Complete Backlog From Current L0 To L1 Closure

This section is a full planning backlog, not a partial starter list. The landing
team may split, merge, or assign these items, but should not skip the acceptance
intent. If the team creates task messages, it should create them from this whole
backlog or from phase-sized batches, not only from the first few rows.

### 12.1 L0 Governance And Hygiene

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-GOV-001 | Create `YUENGINE_L0_COMPLETION_MATRIX.md` | none | Every L0 subsystem has state, owner, next action, required command, allowed skip, and forbidden scope |
| L0-GOV-002 | Create `YUENGINE_BRIDGE_AUDIT.md` | none | Every bridge lists direction, source/destination owner, lifecycle owner, failure statuses, bounds, tests, and risk |
| L0-GOV-003 | Grade hardware skips | L0-GOV-001 | XAudio2/XInput/other hardware skips are classified as acceptable environment skip, required closure blocker, or deferred |
| L0-GOV-004 | Finish sample/repo hygiene | none | No committed generated exe/dll/capture under `Temp`; sample/tool/third-party provenance is documented |
| L0-GOV-005 | Define sample-smoke command | L0-GOV-004 | One documented command builds/runs or intentionally skips the engine sample with explicit status |
| L0-GOV-006 | Freeze new broad gates | none | No new World/Game Adapter/UI/gameplay expansion until L0 matrix and bridge audit are accepted |

### 12.2 L0 Platform And Device Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-PLAT-001 | Confirm Win32 window lifecycle | current Platform first slices | create, resize, focus, close, destroy fixtures pass |
| L0-PLAT-002 | Confirm bounded event pump | L0-PLAT-001 | deterministic drain, close/focus/resize event ordering, no sleep-based proof |
| L0-PLAT-003 | Validate native surface descriptor boundary | L0-PLAT-001 | public value remains opaque; backend consumes through private path |
| L0-PLAT-004 | Add Platform failure-state tests | L0-PLAT-001 | invalid descriptor/double destroy/unsupported mode return explicit statuses |
| L0-PLAT-005 | Feed Platform state into FrameHost | L0-PLAT-001, L0-PLAT-002 | FrameHost consumes Platform events without owning Platform internals |

### 12.3 L0 RHI And D3D11 Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-RHI-001 | Reconfirm backend-neutral device contract | current RHI first slices | capability flags, unsupported states, and handle generations are explicit |
| L0-RHI-002 | Close D3D11 device/swapchain lifecycle | L0-RHI-001, L0-PLAT-003 | create, clear, present, capture, resize, shutdown path works on supported hardware |
| L0-RHI-003 | Close D3D11 resource primitives | L0-RHI-002 | buffer, texture, sampler, shader, input layout, pipeline, and fence/retirement tests pass |
| L0-RHI-004 | Close draw proof set | L0-RHI-003 | visible triangle, indexed static mesh, and texture sampling hardware-smoke pass or are explicitly environment-blocked |
| L0-RHI-005 | Close resize proof | L0-RHI-002 | stale target rejection, backbuffer generation changes, and unsupported resize status are tested |
| L0-RHI-006 | Close RHI failure model | L0-RHI-002, L0-RHI-003 | invalid descriptors, stale handles, unsupported operations, and device-unavailable statuses are deterministic |

### 12.4 L0 RenderCore Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-REN-001 | Stabilize view packet contract | current RenderCore first slices | camera constants, viewport/scissor, frame index, and resize values are explicit |
| L0-REN-002 | Stabilize draw packet contract | L0-REN-001, L0-RHI-003 | geometry, material, shader, and ordering values are caller-owned and bounded |
| L0-REN-003 | Stabilize frame packet execution | L0-REN-002 | prepare/validate/submit path works over RHI values without scene dependencies |
| L0-REN-004 | Stabilize graph skeleton/execution plan | L0-REN-003 | pass declarations and dependency validation pass without scheduler/frame-graph expansion |
| L0-REN-005 | RenderCore failure model | L0-REN-002, L0-REN-004 | missing resources, stale handles, and graph validation failures report explicit status |

### 12.5 L0 File, Package, Resource, Streaming Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-RES-001 | Close File/VFS loose read/write policy | current File first slices | path normalization, mount priority, fixture bounds, and write/read statuses are explicit |
| L0-RES-002 | Close Package load-plan/staging baseline | L0-RES-001 | manifest/load-plan and staging records remain value contracts, not old-package compatibility |
| L0-RES-003 | Close Resource cache/decode chain | current Resource first slices | cache payload, decode plan, decode result, decoded payload ownership, and release behavior pass |
| L0-RES-004 | Close Resource residency/upload chain | L0-RES-003, L0-RHI-003 | upload queue, upload completion commit, residency budget, and stale handle failures pass |
| L0-RES-005 | Close texture bridge to RHI | L0-RES-004, L0-RHI-003 | decoded texture payload maps to upload request without Resource owning RHI lifecycle |
| L0-RES-006 | Close PCM bridge to Audio | L0-RES-003, L0-AUD-002 | decoded audio metadata maps to PCM request without Audio parsing Resource payloads |
| L0-RES-007 | Add sample texture/mesh asset path | L0-RES-001, L0-RES-003, L0-RES-005 | sample asset reaches RenderCore/RHI through YuEngine modules |

### 12.6 L0 Audio Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-AUD-001 | Reconfirm deterministic mixer/test backend | current Audio first slices | existing fast tests remain deterministic and no business BGM/SE IDs appear |
| L0-AUD-002 | Close PCM packet/stream queue | L0-AUD-001 | caller-owned packet records and queue drain pass without hidden allocation |
| L0-AUD-003 | Close XAudio2 callback proof | L0-AUD-002 | supported machine passes hardware smoke; unavailable machine reports explicit skip/status |
| L0-AUD-004 | Audio callback cost proof | L0-AUD-003 | callback path has no file IO, sleeps, or unbounded allocation |
| L0-AUD-005 | Add sample PCM path | L0-AUD-002, L0-RES-006 | sample plays/queues PCM or emits explicit unavailable-device status |

### 12.7 L0 Input Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-IN-001 | Close Win32 keyboard/mouse bridge | current Input first slices | platform events translate to input values with bounded queue behavior |
| L0-IN-002 | Close XInput bridge | current XInput first slice | available/unavailable/connect/disconnect behavior has explicit status |
| L0-IN-003 | Reconfirm replay/action snapshot | L0-IN-001 | action snapshots are deterministic and platform-native types stay private |
| L0-IN-004 | Add sample input path | L0-IN-001, L0-IN-003 | sample consumes input values without UI/gameplay ownership |

### 12.8 L0 Diagnostics, Memory, Thread Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-COST-001 | Reconfirm Memory owner/tag vocabulary | current Memory first slices | owner/tag/counter limitations are documented and tested where available |
| L0-COST-002 | Reconfirm Thread/async completion shutdown | current Thread/File slices | worker lifecycle and async completion shutdown are deterministic |
| L0-COST-003 | Reconfirm Diagnostics disabled neutrality | current Diagnostics slices | disabling diagnostics does not change runtime behavior |
| L0-COST-004 | Add L0 sample cost counters | L0 sample paths | sample emits bounded optional counters for frame/resource/audio/input |
| L0-COST-005 | Add hot-path smoke checks | L0 sample paths | measured fixture paths do not grow containers unexpectedly |

### 12.9 L0 Vertical Sample Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L0-SAMPLE-001 | Create YuEngine-owned sample skeleton | L0-GOV-004 | sample lives under `Samples/`, builds from YuEngine outputs, and has README |
| L0-SAMPLE-002 | Wire Platform + RHI | L0-PLAT-005, L0-RHI-002 | sample creates window/surface/device/swapchain and presents/clears |
| L0-SAMPLE-003 | Wire RenderCore frame | L0-REN-003, L0-SAMPLE-002 | sample submits a bounded view/draw/frame packet through RenderCore |
| L0-SAMPLE-004 | Wire Resource texture/mesh path | L0-RES-007, L0-SAMPLE-003 | sample asset path goes through File/Resource/Streaming/RHI |
| L0-SAMPLE-005 | Wire Input path | L0-IN-004, L0-SAMPLE-003 | sample consumes keyboard/gamepad value snapshots |
| L0-SAMPLE-006 | Wire Audio path | L0-AUD-005 | sample queues/plays PCM or emits explicit unavailable-device status |
| L0-SAMPLE-007 | Wire resize/shutdown | L0-RHI-005, L0-SAMPLE-002 | sample handles resize and shutdown with explicit status |
| L0-SAMPLE-008 | Close Debug/Release/Fast/HardwareSmoke/sample smoke | all L0 sample tasks | required commands pass or skip only documented environment rows |

### 12.10 L1 Governance Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-GOV-001 | Create `YUENGINE_L1_RUNTIME_CORE_MATRIX.md` | L0-GOV-001, L0-SAMPLE-008 or documented blocker | every L1 subsystem has owner, dependencies, forbidden scope, tests, and vertical evidence |
| L1-GOV-002 | Freeze current World bridge expansion | L1-GOV-001 | no new World bridge gates without mapping to runtime vertical closure |
| L1-GOV-003 | Decide L1 vertical sample scope | L1-GOV-001 | sample scene content, required subsystems, and non-goals are documented |

### 12.11 L1 Runtime Kernel Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-KERN-001 | Define runtime app/host | L1-GOV-001, L0-SAMPLE-008 | zero-world runtime loop starts, ticks fixed frames, and shuts down |
| L1-KERN-002 | Define frame phases | L1-KERN-001 | Begin/Poll/LoadCommit/Update/Prepare/Submit/Present/End phases are explicit |
| L1-KERN-003 | Define FrameContext | L1-KERN-002 | frame index, delta/fixed time, input snapshot, diagnostics reference are value contracts |
| L1-KERN-004 | Define runtime error/shutdown propagation | L1-KERN-001 | L0 failure statuses propagate without project-specific behavior |

### 12.12 L1 Object, Component, Transform Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-OBJ-001 | Reconfirm Object identity/lifetime registry | current YuObject | generation handles, acquire/release, retire, and no-mutation failures pass |
| L1-OBJ-002 | Promote component identity baseline | current World component bridges | component type/slot records are bounded and queryable without behavior lifecycle |
| L1-OBJ-003 | Promote transform baseline | current transform bridge | POD transform records are bounded and queryable without hierarchy policy |
| L1-OBJ-004 | Define destroy invalidation path | L1-OBJ-001, L1-OBJ-002, L1-OBJ-003 | object destruction invalidates or explicitly requires caller cleanup for sidecars |
| L1-OBJ-005 | Hot-path object/component query smoke | L1-OBJ-002 | measured query/update fixture has no hidden growth |

### 12.13 L1 World And Scene Assembly Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-WORLD-001 | Reconfirm World lifecycle fixture | current YuWorld | create/update/destroy phases are deterministic |
| L1-WORLD-002 | Define scene assembly record set | L1-OBJ-001, L1-OBJ-002, L1-OBJ-003 | object/transform/component/resource-binding records have explicit ownership |
| L1-WORLD-003 | Reconfirm decoded restore plan/preflight | current P3 scene plan slices | duplicate/missing object/resource statuses are deterministic and no-mutation |
| L1-WORLD-004 | Design active restore coordinator gate | L1-WORLD-003 | validation-before-mutation and rollback/cleanup policy are written before code |
| L1-WORLD-005 | Implement active restore coordinator only after gate | L1-WORLD-004 | active restore applies a validated scene assembly without hidden partial mutation |

### 12.14 L1 Asset Manager Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-ASSET-001 | Define runtime asset handle | L0-RES-003, L1-GOV-001 | stable asset ID/type/generation/state contract exists |
| L1-ASSET-002 | Define asset load states | L1-ASSET-001 | unloaded/loading/decoded/uploaded/resident/failed states are explicit |
| L1-ASSET-003 | Define dependency traversal | L1-ASSET-001 | bounded dependency list and traversal failure statuses exist |
| L1-ASSET-004 | Bind Asset Manager to Resource/Streaming | L1-ASSET-002, L0-RES-004 | asset operations use lower Resource primitives without owning RHI/Audio devices |
| L1-ASSET-005 | Asset fixture path | L1-ASSET-004 | synthetic asset reaches texture/audio ready records and releases cleanly |

### 12.15 L1 Render Scene Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-RSCENE-001 | Define render entity record | L1-OBJ-003, L1-ASSET-001 | object/transform/mesh/material handles are value references |
| L1-RSCENE-002 | Define camera record | L1-KERN-003 | camera values produce RenderCore view packet values |
| L1-RSCENE-003 | Define initial visibility path | L1-RSCENE-001 | full-list traversal works, culling remains deferred |
| L1-RSCENE-004 | Assemble RenderCore packets | L1-RSCENE-001, L1-RSCENE-002, L0-REN-003 | scene records create RenderCore packets without D3D11 includes |
| L1-RSCENE-005 | RenderScene failure states | L1-RSCENE-004 | missing mesh/material/camera returns explicit status |
| L1-RSCENE-006 | Close runtime multi-entity visual frame | L1-RSCENE-004, L0-REN-003, L0-RHI-004, L0-RES-007 | multiple scene entities submit to one runtime frame with distinct transforms, shared material, texture bindings, camera constants, and captured output through YuEngine modules |

### 12.15A L1 Runtime Visual Foundation Backlog

Source document: `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md`.

These rows are the shallow-to-deep module floors for runtime visual closure.
They prevent the team from treating one late sample as a substitute for basic
engine capabilities.

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-CAMERA-001 | Define runtime camera record | L1-KERN-003 | position/orientation or target, FOV, aspect, near/far produce deterministic view/projection values |
| L1-CAMERA-002 | Active camera frame binding | L1-CAMERA-001, L1-RSCENE-004 | RenderScene frame references one active camera record and rejects missing camera |
| L1-CAMERA-003 | Camera capture metadata | L1-CAMERA-002, L0-RHI-002 | runtime capture records frame index, camera pose, capture target/status, and bounded output metadata |
| L1-GEOM-001 | Runtime primitive geometry records | L0-REN-003, L1-ASSET-001 | cube/cylinder/cone geometry records expose bounded vertex/index/draw ranges |
| L1-MAT-001 | Runtime material texture-slot record | L0-REN-004, L1-ASSET-005 | one material binds at least three texture inputs by slot with explicit missing/invalid statuses |
| L1-ANIM-001 | Animation clip/track/keyframe records | L1-KERN-003 | bounded multi-clip runtime records and explicit selected clip ids exist without editor timeline or gameplay dependency |
| L1-ANIM-002 | Interpolation sampler | L1-ANIM-001 | deterministic scalar/vector/rotation or transform interpolation tests pass at fixed times |
| L1-ANIM-003 | Runtime time sampling | L1-ANIM-002 | `FrameContext` time samples tracks without hidden global time |
| L1-ANIM-004 | Apply sampled transform | L1-ANIM-003, L1-OBJ-003 | sampled output, including explicitly selected clip output, updates object transform records before RenderScene consumes them |
| L1-ANIM-005 | Animation failure states | L1-ANIM-001 | missing selected clip, missing clip/track, unsupported interpolation, out-of-range time, and capacity overflow return explicit statuses without output mutation |
| L1-VIS-001 | Static one-cube capture | L1-CAMERA-003, L1-GEOM-001, L1-MAT-001 | checked-in runtime command captures one cube through YuEngine modules |
| L1-VIS-002 | Three-primitive placed scene | L1-VIS-001, L1-RSCENE-006 | cube/cylinder/cone submit as three entities with fixed-seed transforms |
| L1-VIS-003 | Shared three-texture material scene | L1-VIS-002, L1-MAT-001 | all primitives use one material with three distinct texture inputs |
| L1-VIS-004 | Animated transform scene | L1-VIS-002, L1-ANIM-004 | object rotations are driven by runtime animation, including explicit selected clip output, or the command reports `L1-ANIM-*` blocker |
| L1-VIS-005 | Camera tween capture sequence | L1-VIS-003, L1-VIS-004 | explicit perspective camera tween keyframes emit bounded frame/capture set |
| L1-VIS-006 | Missing-layer diagnostic route | L1-VIS-005 | failure names the exact missing layer: camera, geometry/model, material, shader/pipeline, scene placement, animation, RenderScene, RenderCore/RHI, capture, or resource resolution; shader/pipeline value-contract evidence includes unsupported backend and invalid input-layout prevalidation before RHI mutation |

### 12.16 L1 Audio Scene Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-ASCENE-001 | Define sound asset binding | L1-ASSET-001, L0-AUD-002 | sound handles bind to Audio value contracts |
| L1-ASCENE-002 | Define audio source record | L1-ASCENE-001 | play/stop/pause/gain/loop values are explicit |
| L1-ASCENE-003 | Define minimal bus routing | L1-ASCENE-002 | fixed internal bus IDs route to mixer/test backend |
| L1-ASCENE-004 | Submit audio source updates | L1-ASCENE-002, L0-AUD-005 | source state maps to PCM stream queue request or explicit unavailable status |
| L1-ASCENE-005 | AudioScene failure states | L1-ASCENE-004 | missing asset/backend produces explicit status |

### 12.17 L1 Input Mapping Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-INPUT-001 | Define action map | L0-IN-003 | action ID, device binding, button/axis values are explicit |
| L1-INPUT-002 | Define input context | L1-INPUT-001 | active context/focus mode values are explicit |
| L1-INPUT-003 | Produce runtime command snapshot | L1-INPUT-001, L1-KERN-003 | pressed/released/held/axis values are frame-indexed and replayable |
| L1-INPUT-004 | Input mapping tests | L1-INPUT-003 | keyboard and XInput fixtures pass or skip only by documented hardware state |

### 12.18 L1 Serialization, Save, Config Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-SER-001 | Reconfirm value stream contract | current YuSerialize | caller-provided buffer, version, unsupported status, and no-mutation failures pass |
| L1-SER-002 | Serialize scene assembly records | L1-WORLD-002, L1-SER-001 | object/transform/component/resource-binding records roundtrip deterministically |
| L1-SER-003 | Define runtime config record | L1-KERN-001, L1-SER-001 | config serializes without File/Package dependency inside Serialize core |
| L1-SER-004 | Define save/profile boundary | L1-SER-002 | persistence policy stays outside Serialize core and original-save compatibility |

### 12.19 L1 Script Native Bridge Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-SCRIPT-001 | Reconfirm native call registry | current YuScript | stable call IDs and caller-owned value slots pass |
| L1-SCRIPT-002 | Runtime phase dispatch adapter | L1-WORLD-001, L1-SCRIPT-001 | World phase trace maps to call IDs without World core depending on Script core |
| L1-SCRIPT-003 | Script error/failure states | L1-SCRIPT-001 | missing call/invalid slots return explicit status |

### 12.20 L1 Runtime Diagnostics Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-DIAG-001 | Define runtime counters | L1-KERN-003 | frame/object/resource/render/audio/input counters are bounded and optional |
| L1-DIAG-002 | Diagnostics-disabled equivalence | L1-DIAG-001 | runtime behavior is identical when diagnostics are disabled |
| L1-DIAG-003 | Debug overlay hook proposal | L1-DIAG-001 | hook is optional tooling plane, not runtime dependency |

### 12.21 L1 Vertical Sample Backlog

| ID | Work item | Depends on | Acceptance |
| --- | --- | --- | --- |
| L1-SAMPLE-001 | Define synthetic scene manifest | L1-WORLD-002, L1-ASSET-001 | manifest is project-independent and does not parse old package runtime |
| L1-SAMPLE-002 | Boot runtime and create world | L1-KERN-001, L1-WORLD-001 | runtime creates world and enters fixed-frame loop |
| L1-SAMPLE-003 | Instantiate objects/components/transforms | L1-OBJ-005, L1-SAMPLE-002 | sample object graph is deterministic |
| L1-SAMPLE-004 | Bind resources/assets | L1-ASSET-005, L1-SAMPLE-003 | texture/audio handles bind through Asset Manager |
| L1-SAMPLE-005 | Map input to command snapshot | L1-INPUT-004, L1-SAMPLE-002 | input affects sample state only through runtime command values |
| L1-SAMPLE-006 | Submit render scene | L1-RSCENE-005, L1-SAMPLE-004 | sample produces RenderCore/RHI frame through L1 RenderScene |
| L1-SAMPLE-007 | Submit audio scene | L1-ASCENE-005, L1-SAMPLE-004 | sample submits audio or explicit unavailable status |
| L1-SAMPLE-008 | Serialize and reload snapshot | L1-SER-004, L1-SAMPLE-003 | sample state roundtrips through value streams without File/Package dependency in Serialize |
| L1-SAMPLE-009 | Shutdown and cleanup proof | all L1 sample tasks | world/runtime/resources shut down with no leaked active records |
| L1-SAMPLE-010 | Debug/Release/Fast validation | all L1 sample tasks | fast gate plus sample smoke pass; release validation command is documented |
| L1-SAMPLE-011 | Runtime visual scene proof | L1-VIS-005, L1-RSCENE-006, L1-ASSET-005, L1-OBJ-003 | cube/cylinder/cone scene renders with deterministic placement, explicit selected-clip animation-driven or explicitly blocked per-object rotation, shared three-texture material, explicit perspective camera tween, shader/reflection value-contract proof, and bounded capture set |
| L1-SAMPLE-012 | Runtime visual blocker report | L1-SAMPLE-011 | if the visual scene cannot run, output names the missing layer exactly: camera, geometry/model path, material texture slots, shader/pipeline, scene placement, animation interpolation, selected animation clip, transform application, RenderScene multi-entity, RenderCore multi-draw, RHI capture, camera tween sampling, or resource resolution; shader/pipeline failures must surface exact unsupported backend or invalid input-layout status instead of mutating RHI output |

## 13. Guardrails

Stop and return to architecture if any of these happen:

- A lower module starts depending on World, UI, Script, or Project types.
- A bridge starts owning both sides' lifecycle.
- A report/log/capture becomes required for runtime behavior.
- A sample bypasses YuEngine modules with standalone demo code.
- A test uses sleeps/manual screenshots/manual audio listening as primary proof.
- A hardware skip is counted as done without grading.
- A project-specific need changes L0/L1 API without a gate.
- Old TouhouNewWorld package compatibility appears in L0 or L1.
- New World bridge work continues before L0 closure matrix is accepted.

## 14. Summary For Landing Team

The project is not behind enough to roll back. The project is ahead in several
module proofs but needs execution discipline now.

The correct move is not to add more broad gates. The correct move is:

```text
Matrix -> Bridge Audit -> L0 Engine Sample -> L1 Runtime Matrix -> L1 Vertical Sample
```

Keep YuEngine narrower than UE/Unity, but not weaker in production discipline.
Avoid developer ecosystem work. Preserve clean ownership, deterministic tests,
failure semantics, and performance budgets.
