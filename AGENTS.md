# YuEngine Agent Handoff

这份文件是给后续 AI agent 看的第一入口。新 agent 如果打开
`C:\Steam\steamapps\common\TouhouNewWorld\YuEngine` 作为根目录，先读这里。

## Root

- 当前正式 git 仓库是 `YuEngine`。
- 后续长期工程、可提交代码、可维护计划和工具，优先落在本仓库内。
- 外层 `TouhouNewWorld` 是材料区，不是正式工程根。
- 旧 `Project2` 是前置准备区，里面已有计划、研究输出和工具；后续应逐步把仍然有效的内容迁移或重建进 `YuEngine`。
- 旧 `Project/reimplementation` 不能作为实现基础，只能作为反例或材料。

## Target

目标不是重建一个只能跑 Touhou New World 的硬编码壳。

目标是从 Touhou New World 的资源、脚本、二进制线索和运行行为中，抽出一个可复用的商业自研游戏引擎 runtime。原游戏是第一个 sample project 和 behavioral oracle，用来证明 runtime 的真实行为。

第一阶段不需要编辑器。第一阶段要做到：

```text
project.json
-> project boot
-> VFS/resource mount
-> script VM
-> native/API service registry
-> title menu driven by original script
-> save/new-game/profile flow
-> scene load
-> actor/player/camera/input
-> early mission/tutorial flow
-> renderer/audio/save baseline
```

后期再补 CLI 工具链、打包、导表、资源依赖图、场景/模型/材质/脚本编辑器，并逐步演进到 Unity/UE 式高编辑度和先进生产管线。

## Current State

`YuEngine` 仓库刚建立，目前只有 `LICENSE` 和本文件。

前置准备主要在 sibling path：

```text
..\Project2
```

已有准备包括：

- 长期根计划：`..\Project2\docs\ENGINE_EXTRACTION_PLAN.md`
- 运行时启动规格：`..\Project2\docs\PROJECT_RUNTIME_SPEC.md`
- native/API 服务归属：`..\Project2\docs\ENGINE_SERVICE_MAP.md`
- 游戏运行和生产模型：`..\Project2\docs\GAME_RUNTIME_AND_PRODUCTION_MODEL.md`
- 原游戏 sample/oracle 验收计划：`..\Project2\docs\REBUILD_PLAN.md`
- 前置准备门禁：`..\Project2\docs\PRE_RECONSTRUCTION_READINESS.md`
- 自动化还原流程：`..\Project2\docs\PERFECT_RECONSTRUCTION_AUTOMATION.md`
- AI 加速计划：`..\Project2\docs\AI_NATIVE_ACCELERATED_PLAN.md`
- oracle trace 准备：`..\Project2\docs\ORACLE_TRACE_PREP.md`
- script IR 准备：`..\Project2\docs\SCRIPT_IR_PREP.md`
- 当前前线状态：`..\Project2\research\RECONSTRUCTION_FRONTIER.md`

已有工具：

```powershell
python ..\Project2\tools\evidence_indexer.py
python ..\Project2\tools\evidence_query.py summary
python ..\Project2\tools\evidence_query.py script script/menu/titlemenu.b64.sqasm
python ..\Project2\tools\entrypoint_closure.py script/menu/titlemenu.b64.sqasm
```

已有研究输出：

- `..\Project2\research\evidence_graph.sqlite`
- `..\Project2\research\evidence_graph.jsonl`
- `..\Project2\research\evidence_manifest.json`
- `..\Project2\research\native_binding_inventory.md`
- `..\Project2\research\query_examples.md`
- `..\Project2\research\entrypoints\script_menu_titlemenu_closure.md`
- `..\Project2\research\entrypoints\mission_ms0915_closure.md`
- `..\Project2\research\entrypoints\mission_ms0916_closure.md`
- `..\Project2\research\entrypoints\mission_sc01_main_ms010_0_closure.md`

## Evidence Baseline

当前索引基线：

- 13,780 resources
- 76 audio tracks
- 859 Squirrel scripts
- 31,138 Squirrel functions
- 2,189,098 Squirrel instructions
- 246,975 script call sites
- 66,318 engine/native boundary candidate call sites
- 24,994 binary functions
- 600 RTTI names
- 157 source path hints

这些数字来自 `..\Project2\tools\evidence_indexer.py`。如果新 agent 继续工作，先用 query 工具复查，不要靠手动搜索猜。

## Material Map

从 `YuEngine` 根目录看，主要材料位置是：

```text
..\Project2                  前置计划、研究输出、索引工具
..\Project                   旧材料库；不要继承旧 runtime
..\Project\output            已提取资源、脚本反汇编、Ghidra/RTTI/source-path 等输出
..\resource                  原游戏资源和 pack
..\data                      原游戏数据
..\texts                     原游戏文本
..\yufulao\AGENTS.md         用户长期迭代出的工程规范
..\yufulao\skills-main       agent 工程流程规范来源
..\yufulao\framework         用户 Unity 自研框架
..\yufulao\framework2        用户 Unity 自研框架第二套材料
..\yufulao\system-big        大型商业游戏系统设计和部分源码材料
D:\app\Epic Games\UE_5.5\Engine  UE5 源码材料
```

这些都是工程输入材料，可以读、比较、迁移、改写、适配、集成。它们不是额外 skill，也不是用来限制 agent 的壳。

落进 `YuEngine` 的内容必须满足：

- 理解后再迁移；
- 命名适配当前 engine；
- 接入当前 lifecycle；
- 有明确 service owner；
- 可编译、可运行、可验证。

## Known Frontline Chains

### Title/Menu

Entrypoint:

```text
script/menu/titlemenu.b64.sqasm
```

已验证链：

```text
titlemenu.b64
-> TitleScene / NewGameScene / LoadScene / OverwriteSaveScene
-> GetSaveList / LoadAutoSave
-> GetScenarioKeys / FindScenario / SetDifficultyMode
-> MakeNewGame
-> StartGame
```

关键资源：

```text
menu/title/title_back
-> menu/title/title_back_en.dds
-> menu/title/title_back_jp.dds
-> menu/title/title_back_sc.dds
-> menu/title/title_back_tc.dds
```

相关 calls:

```text
GetSaveList
LoadAutoSave
GetScenarioKeys
FindScenario
SetDifficultyMode
MakeNewGame
StartGame
PlayBGM
PlaySE
GetGraphResident
DrawGraph
DrawString
GraphString
GraphParam
```

### Mission/Scene

Entrypoint candidate:

```text
mission/sc01/main/ms010_0.b64.sqasm
```

已验证链：

```text
mission/sc01/main/ms010_0.b64
-> setupProcess
-> LoadStage("map/Doujou/doujou.sge")
-> LoadEventsScriptViaMission
-> CallSetupEvents
-> PushPlayerChara
-> PushTaskGameCamera
```

关键资源：

```text
map/Doujou/doujou.sge -> map/doujou/doujou.sge
map/Doujou/doujou.rcm -> map/doujou/doujou.rcm
```

相关 calls:

```text
Loader
LoadStage
LoadEventsScriptViaMission
CallSetupEvents
PushPlayerChara
PushTaskGameCamera
LoadRailCamera
SetEnableRailCamera
SetEnableAutoCameraAdjust
SetDefaultCameraState
LoadCharaPlace
GetPlaceParams
GetMarkerFromRequest
EventClass
EventVolume
GetEventUnit
SetPlayerControl
ActorTutorial
```

## Unknowns That Matter

不要在这些问题没证据时直接写死：

- `StartGame`: profile 输入、mission 选择输出、scene transition 副作用。
- `MakeNewGame`: save/profile mutation shape。
- `GetSaveList`: return table schema、空/非空行为。
- `GetScenarioKeys` / `FindScenario`: scenario 数据来源、返回对象 shape。
- `LoadStage`: stage resource graph、renderer state、collision/camera dependencies。
- `PushPlayerChara`: actor handle、player resource selection、placement semantics。
- `PushTaskGameCamera`: camera task lifecycle、initial state。
- `ActorTutorial`: tutorial actor/page 行为、完成 flag。
- `GetFlag` / event flags: flag storage、mission gating。

这些要靠 oracle sampling、静态交叉证据、Script IR 和 native boundary spec 解决。

## Evidence Levels

用证据等级防止虚假进度：

```text
E0: indexed artifact only
E1: single-domain interpretation
E2: cross-domain support
E3: oracle-sampled behavior
E4: Project2/YuEngine implementation verified
E5: oracle-diff accepted
```

规则：

- E0/E1 只能作为线索。
- E2 可以设计 module interface。
- E3 才能声称 native behavior 被理解。
- E4 才能声称模块已实现。
- E5 才能声称 sample-project flow 已还原。

## Completion Levels

长期完成等级：

```text
R0: Evidence map
R1: Project boot
R2: Scripted title
R3: Save and New Game
R4: Scene entry
R5: First playable flow
R6: Generic project proof
R7: Production without editor
R8: Editor and advanced pipeline
```

当前应视为 R0 准备阶段附近：证据图、entrypoint closure、native inventory 已有，但 oracle trace、Script IR 工具、native status table 仍需继续完成。

## Milestones

长期 gate：

```text
X0: Evidence-To-Engine Map
X1: Project Boot Runtime
X2: Title/Menu As Sample Project
X3: New Game And Profile Services
X4: Scene Runtime
X5: First Playable Flow
X6: Generic Project Proof
X7: Production Without Editor
X8: Editor And Advanced Pipeline Later
```

不要跳阶段。后面阶段可以做 throwaway prototype，但不能算完成。

## Current Next Tasks

优先任务不是写游戏窗口。

当前下一步：

1. 把仍然有效的 `Project2` 计划、工具和研究输出整理进 `YuEngine` 仓库结构。
2. 建立 `YuEngine` 内的 `docs/`、`tools/`、`research/`、`src/`、`tests/`、`samples/` 目录。
3. 保留 `Project2` 作为前置材料，不再让正式工程继续散在外层目录。
4. 做 P1: Oracle Title Boot。
5. 做 P2: Oracle New Game。
6. 做 P3: Script IR Tool。
7. 做 P4: Native Boundary Spec Table。
8. 做 P5: Generic Project Manifest。
9. 做 P6: Engine API Surface Map。

Do not start runtime implementation until P1-P4 have enough signal for the target slice.

## Do Not Repeat Previous Failure

之前失败路线：

- 手写临时主菜单；
- 蓝底场景；
- 静态 mesh 能看但流程不对；
- T pose 角色；
- camera 不受脚本/native 控制；
- new game / save / tutorial 流程乱；
- 缺什么补什么，没有 bottom-up runtime foundation。

当前路线必须反过来：

```text
evidence
-> oracle
-> Script IR
-> native/API service ownership
-> project runtime foundation
-> script-driven menu
-> save/new-game
-> scene/actor/camera
-> first playable flow
```

## Runtime Services

每个 script-visible boundary 必须归属一个 service：

- Project Service
- Platform Service
- Resource Service
- Script Service
- UI And 2D Render Service
- Save/Profile/Scenario Service
- Scene And Stage Service
- Actor And Task Service
- Camera Service
- Collision And Physics-Lite Service
- Audio Service
- Event/Quest/Flag Service

每个 API row 最终状态：

```text
candidate
-> service owner
-> script evidence
-> oracle/static evidence
-> implementation
-> diff pass
```

service owner 未知时，不要开始实现。

## Multi-Agent Work Model

支持多 agent 并行，但必须分 lane，避免互相污染。

推荐 lane：

- `evidence`: evidence graph、query、entrypoint closure、manifest。
- `oracle`: original game trace、file IO、D3D9、save/input sampling。
- `script-ir`: `.sqasm` -> stable JSON IR、state machine、resource refs。
- `native-spec`: native/API status table、argument/return shape、service owner。
- `project-runtime`: `project.json`、boot contract、VFS mount。
- `script-runtime`: Squirrel VM、module loader、native registry。
- `ui-render`: GraphHandle、DrawGraph、DrawString、font/DDS command buffer。
- `save-scenario`: save/profile/scenario/new-game/start-game services。
- `scene-world`: stage loader、event script, scene transition。
- `actor-camera`: actor handle, player spawn, task, camera, input flags。
- `render-audio`: D3D-compatible renderer, material/model/effect/audio.
- `toolchain`: validator, pack builder, table validator, dependency graph.

并行规则：

- 每个 agent 先读本文件，再读当前 lane 相关计划和 evidence。
- 每个 agent 开工前运行 `git status --short --branch`。
- 不要覆盖其他 agent 的未提交改动。
- 多 agent 并行时优先使用独立 branch，例如 `agent/oracle-title-boot`。
- 每个 agent 只修改自己 lane 的文件，跨 lane 修改要在最终汇总时处理。
- 每个 lane 的产出必须有命令、trace、test 或 query 证明。
- 如果没有验证路径，先建验证路径，再写实现。

## Git Rules

- `YuEngine` 是正式 repo。
- 改动前后检查 `git status`。
- 不要在外层目录留下正式工程代码。
- 不要自动 push。需要 push 时明确告诉用户。
- 不要 reset、checkout、clean 用户改动。
- 如果遇到不属于自己的改动，保留并绕开；如果影响当前任务，先说明冲突。

## Engineering Rules

通用规则已经从用户的 `AGENTS.md` 和 `skills-main` 提炼过：

- 商业项目质量：性能、维护性、拓展性、稳定性同时考虑。
- 先证据，后实现。
- 先建立反馈回路，再调试或实现。
- 修 bug 走 reproduce -> minimize -> hypotheses -> instrument -> fix -> regression。
- 测试行为，不测内部形状。
- 垂直切片，不做横向堆文件。
- 追求 deep modules：小 interface，大 implementation，高 leverage。
- silent no-op 禁止。
- 残留 mismatch 必须命名。
- 临时 debug log 使用 `[DEBUG-...]` 前缀，结束前删除。

Unity/C# 相关材料如果被使用，遵守：

- one file one type；
- early return；
- no `else`；
- no LINQ；
- foreach only；
- namespace 放 top-level using；
- UI declared components 直接使用，不乱判空；
- non-log string constants；
- manager lifecycle: `OnInit`, `Update`, `FixedUpdate`, `LateUpdate`, `OnClear`。

## Safety And Anti-Goals

- 不做 Steam/DRM bypass。
- Steam/login/entitlement 可以建成本地可替换 Platform Service stub。
- 不声称恢复了原厂 C++ 源码，除非有 source/PDB/object evidence。
- 当前目标是兼容 runtime source，不是伪装成原始私有仓库。
- 不把 `map/Doujou`、`mission/sc01/main/ms010_0`、title menu state 直接硬编码进 engine。
- 不用手写 UI 代替 `titlemenu.b64`。
- 不用 mesh preview 冒充 scene/runtime。

## First Commands For A New Agent

从 `YuEngine` 根目录开始：

```powershell
git status --short --branch
Get-Content -Raw AGENTS.md
Test-Path ..\Project2
Get-Content -Raw ..\Project2\research\RECONSTRUCTION_FRONTIER.md
python ..\Project2\tools\evidence_query.py summary
python ..\Project2\tools\evidence_query.py script script/menu/titlemenu.b64.sqasm
python ..\Project2\tools\entrypoint_closure.py script/menu/titlemenu.b64.sqasm
```

如果 sibling `..\Project2` 不可访问，先不要继续猜。需要把历史准备材料导入 `YuEngine`。

## Implementation Trace Shape

每个模块或 native/API 实现必须能回答：

```text
Module:
Interface:
Service owner:
Engine layer:
Sample-project flow:
Evidence:
  Scripts:
  Resources:
  Native/API calls:
  Binary/static:
  Oracle traces:
Inputs:
Outputs:
Side effects:
Unknowns:
Verification:
Residual mismatches:
```

这不是文档仪式。它是避免后续 agent 胡写、重写、互相污染的最低上下文。
