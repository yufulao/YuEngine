# YuEngine Agent Handoff

这份文件是给后续 AI agent 看的第一入口。新 agent 如果打开
`C:\Steam\steamapps\common\TouhouNewWorld\YuEngine` 作为根目录，先读这里。

## Root

- 当前正式 git 仓库是 `YuEngine`。
- 后续长期工程、可提交代码、可维护计划和工具，优先落在本仓库内。
- 外层 `TouhouNewWorld` 是材料区，不是正式工程根。
- 旧 `Project2` 是前置准备区，里面已有计划、研究输出和工具；迁移到 `YuEngine` 前必须先做审计和筛选，不能批量复制。
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

## Corrected Execution Policy

用户已反复明确纠正：不要把“最小验证”“最小入口”“小工具”当成工程阶段完成。

当前执行方式改为完整引擎主干优先：

```text
C++ runtime spine
-> project/runtime lifecycle
-> VFS/resource system
-> script module/VM layer
-> native/API service registry
-> original sample project boot
-> script-driven title/new-game/scene
-> actor/camera/input/tutorial flow
```

当前最新硬约束：

- 不允许把任何“最小可运行”“最小入口”“最小 VM”“最小菜单”“最小场景”作为默认策略或阶段终点。
- 不允许把“先做一个 tiny launcher / tiny VM / tiny renderer 再慢慢替换”作为计划主线。
- 允许为了降低风险切出可验证的子边，但这个子边必须属于完整 runtime contract，完成后立即接下一条边。
- 每个提交只是循环 checkpoint，不是停止理由。提交后继续推进下一个非阻塞 contract edge。
- 文档、计划、readiness、统计、trace、binding report、CLI 输出只算证据输入；必须继续进入 C++ runtime/service/VM 行为。
- 若当前阶段还没达到用户目标：原脚本驱动主菜单、存档/新游戏、进场景、角色/相机/教程流程，就不能把阶段说成完成。
- agent 不得用“先做个小 demo 看看”来替代工程拆解。正确拆法是 layer contract -> evidence -> service/API -> runtime implementation -> regression test -> residual unknowns。
- 严禁“缺什么补什么”的横向试探路线。每个循环必须从当前层 contract 出发，向下补 evidence/API/service，向上接 sample project behavior。

允许在证据不足时启动 runtime 主干建设，但必须遵守：

- 未确认 native 行为必须是显式 obligation，不能 silent no-op。
- 未采样 oracle 的行为不能声称 original-correct。
- 可运行诊断不等于游戏可运行。
- 不能再以蓝底、手写菜单、mesh preview 或 T pose 作为进度。
- 每轮完成一个任务后自动进入下一任务，除非用户中断或没有非阻塞任务。
- 计划、报告、readiness、统计表和解析器只是工程输入，不是可停点；提交 checkpoint 后继续把它接入 runtime contract。
- 接力 agent 不允许把“先做一个最小验证”作为默认策略。默认策略是补齐当前层的完整证据链、接口、实现、验收和残留未知。

## Non-Stop Loop Rule

如果用户说“继续”“落地”“自己 plan”“token 随便花”，agent 的行为必须是：

```text
read current state
-> choose the next non-blocked layer edge
-> implement runtime/tooling contract
-> verify
-> document exact unknowns
-> commit
-> continue immediately
```

禁止在以下状态停止：

- 只修了规范或计划；
- 只跑了 build/test；
- 只新增了分析报告；
- 只证明某个 entry/binding/call 能解析；
- 只把 native/API 标出来但没有接 service dispatch；
- 只实现了一个临时可视化窗口；
- 只输出“下一步建议”但当前下一步可以自己执行。

停止只允许发生在：

- 用户明确要求暂停或改目标；
- 完整目标已达成；
- 同一硬 blocker 已连续三轮确认，且没有其他非阻塞工程边可以推进。

## Project Failure Correction

旧 `Project` 的失败不是资源不足，而是工程路线错误：从可见缺口反向补 UI、mesh、camera、
actor，最后得到的是互相不受约束的碎片。`YuEngine` 的工作单位必须反过来定义：

```text
engine layer contract
-> evidence/oracle/script/resource inputs
-> service interface
-> runtime implementation
-> diagnostics/tests
-> sample project behavior
```

禁止把以下内容当成阶段成果：

- 手写临时菜单或蓝底窗口；
- 只为当前画面补一个 native stub；
- 只会 preview mesh、不会进入脚本驱动流程；
- actor/camera/input/tutorial 不受原脚本和 service state 控制；
- 只有 CLI 统计，没有进入下一层 runtime contract；
- 只有 plan/diagnostic/readiness，没有继续进入执行层；
- 为了“先跑起来”绕开 `project.json`、VFS、Script Service、Native Service。

允许做工具和诊断，但它们必须服务于主干 runtime，并且完成后自动进入下一层实现。

## Current State

`YuEngine` 仓库已经进入第一轮落地状态，不再是空仓库。

当前已提交的本仓库内容：

- `AGENTS.md`: agent handoff 和长期工程约束。
- `docs/ENGINE_RECONSTRUCTION_MASTER_PLAN.md`: 修正后的完整引擎重建总计划。
- `docs/LOOP_TASKS.md`: 长循环任务队列，新 agent 应按队列持续推进。
- `docs/IMPORT_AUDIT.md`: `Project2` 材料迁移审计。
- `docs/SCRIPT_IR_STATUS.md`: P3 Script IR 当前状态、验收和残留缺口。
- `docs/NATIVE_BOUNDARY_SPEC_STATUS.md`: P4 native/API boundary spec 当前状态和残留缺口。
- `docs/native_boundary_spec/title_first_mission.md`: title/new-game/first-mission boundary baseline，84 APIs、303 call sites、0 unassigned owners。
- `docs/ORACLE_TITLE_BOOT_STATUS.md`: P1 oracle title boot 准备状态和残留缺口。
- `docs/oracle/title_boot_runbook.md`: 原游戏 title boot 采样步骤；不包含 bypass。
- `docs/PROJECT_MANIFEST_STATUS.md`: P5 project manifest 当前状态和残留缺口。
- `docs/API_SURFACE_STATUS.md`: P6 engine API surface 当前状态和残留缺口。
- `docs/engine_api_surface/title_first_mission.md`: title/new-game/first-mission API surface baseline，11 services、84 APIs、303 call sites。
- `docs/RUNTIME_SPINE_STATUS.md`: L1 C++ runtime spine 当前状态、验证命令和边界。
- `docs/VFS_RESOURCE_STATUS.md`: L2 VFS/resource dependency diagnostics 当前状态、验证命令和边界。
- `docs/SCRIPT_MODULE_STATUS.md`: L3 C++ script module model 当前状态、验证命令和边界。
- `docs/NATIVE_SERVICE_INTERFACES_STATUS.md`: L4 C++ native service interfaces 当前状态、验证命令和边界。
- `docs/SERVICE_BACKED_RUNTIME_STATUS.md`: L5 service-backed runtime lifecycle 当前状态、验证命令和边界。
- `docs/ORACLE_CAPTURE_EXECUTION_STATUS.md`: L6 oracle capture readiness 当前状态、验证命令和边界。
- `docs/TITLE_SCRIPT_EXECUTION_STATUS.md`: L7 title script execution 当前进度、验证命令和剩余缺口。
- `docs/SCENE_ENTRY_RUNTIME_STATUS.md`: L8 scene-entry runtime contract 当前进度、验证命令和剩余缺口。
- `docs/SCENE_RUNTIME_MATERIALIZATION_STATUS.md`: L9 scene runtime materialization 当前进度、验证命令和剩余缺口。
- `docs/FIRST_FRAME_RUNTIME_STATUS.md`: L10 first-frame renderer/input/event contract 当前进度、验证命令和剩余缺口。
- `CMakeLists.txt`: C++20 runtime/CLI build 和 CTest 验收入口。
- `src/yuengine/...`: core JSON、project manifest、VFS/resource diagnostics、`.sqasm` diagnostics、native registry、native service catalog、runtime boot report。
- `apps/yuengine_cli`: `validate`、`boot`、`resources`、`script` 和 `native-services` CLI。
- `samples/touhou_new_world/project.json`: 原游戏作为 sample/oracle project 的 YuEngine manifest。
- `samples/empty_project/project.json`: 不依赖原游戏资源的 generic sample manifest。
- `tools/sqir.py`: `.sqasm` -> JSON IR / state-machine summary 工具，可选接入 `Project2` evidence graph。
- `tools/native_spec.py`: 从 Script IR/evidence 生成 native/API boundary spec 的工具。
- `tools/oracle_title_boot.py`: 原游戏 title boot 的 inspect/snapshot/runbook/显式 launch 准备工具。
- `tools/project_manifest.py`: `project.json` validator。
- `tools/api_surface.py`: 从 Script IR/native boundary spec 生成 engine API surface map。
- `tests/test_sqir.py`: Script IR parser smoke test。
- `tests/test_native_spec.py`: 关键 service owner 规则 smoke test。
- `tests/test_oracle_title_boot.py`: oracle title boot inspect/snapshot smoke test。
- `tests/test_project_manifest.py`: project manifest validator smoke test。
- `tests/test_api_surface.py`: API surface service module naming smoke test。

生成出的 Script IR 默认写到：

```text
build/script_ir
```

该目录被 `.gitignore` 排除，不作为源码提交。

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

工具副作用规则：

- `evidence_query.py` 当前视为只读查询工具，适合第一轮校准。
- `entrypoint_closure.py` 会刷新 `..\Project2\research\entrypoints\...` 输出，不属于第一轮校准命令。
- `evidence_indexer.py` 会重建 evidence graph，不属于第一轮校准命令。
- oracle tracer、code generator、status table generator、closure generator 都默认视为有写入副作用。
- 如果不确定工具是否会写文件，先读源码或用 `--help` 检查，再决定是否运行。

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

## First-Turn Protocol

新 agent 第一次接力时，默认目标是校准并接入当前主干，不是重新发明路线，也不是做最小 demo。

收到一句“继续”或“按 AGENTS 做”时，只允许做这些事：

- 读取 `AGENTS.md`。
- 运行 `git status --short --branch`。
- 读取 `..\Project2\research\RECONSTRUCTION_FRONTIER.md`。
- 运行一到三个只读 `evidence_query.py` 查询验证前线基线。
- 选择一个 lane，说明下一步可进入主干的完整层级闭环。
- 如果要改文件，改动必须落到 `YuEngine` 主干 contract、实现、测试或文档之一；不限单文件，但必须有明确验收。

第一次接力禁止做这些事，除非用户明确要求：

- 修改 sibling `..\Project2`、外层 `TouhouNewWorld` 或其他材料目录。
- 运行会写输出的工具，包括 `entrypoint_closure.py`、`evidence_indexer.py`、oracle tracer、code generator、status table generator。
- 批量复制 `..\Project2` 到 `YuEngine`。
- 批量创建 `docs/`、`research/`、`tools/`、`samples/`。
- 把 generated artifacts 放进 git。
- 把 service-owner tracking 表说成 P4/P6 已完成。
- 把“0 unowned”说成 native/API 行为已经确认。
- 改写 `AGENTS.md` 的 Current State，让仓库看起来已经完成正式迁移。
- 新建手写菜单、mesh preview、蓝底场景或其他绕开脚本/runtime 主干的可视化壳。

## Migration Policy

`Project2` 的内容可以迁移，但必须按以下顺序：

```text
audit
-> select
-> import one category
-> verify commands still work
-> update AGENTS.md current state only after verification
```

迁移分类：

- `source-of-truth docs`: 可以优先迁移，但要保留与 `Project2` 的关系说明。
- `tools`: 可以迁移，但迁移后必须能从 `YuEngine` 根目录运行。
- `samples`: 可以迁移，但路径必须可解释，不能偷偷依赖错误 cwd。
- `research summaries`: 可以迁移小型摘要、frontier、inventory、closure markdown。
- `generated heavy artifacts`: 默认不迁移。

Generated artifacts 包括：

- `evidence_graph.sqlite`
- `evidence_graph.jsonl`
- 大型 closure JSON
- trace 文件
- screenshots/frame dumps
- apitrace/PIX capture
- 自动生成的 status table 输出

这些默认留在 `Project2` 或 runtime output/cache 目录。要进入 git，必须说明为什么源码仓库需要它，而不是只保留生成命令。

## Status Table Policy

Native/API status table 是 tracking artifact，不是行为还原证明。

Service owner 有三种等级：

```text
proposed_owner     来自名称规则、service map 或脚本语境
supported_owner    至少有两类证据支持
accepted_owner     已被 oracle/static evidence 验证并用于实现
```

`0 unowned` 只表示所有行都有 proposed owner，不能表示：

- native 已确认；
- 参数/返回值已确认；
- 副作用已确认；
- P4 已完成；
- 可以开始 runtime 实现。

P4 完成至少需要：

- boundary list 覆盖 title/newgame/first-mission；
- 每行有 owner 等级；
- confirmed native 状态；
- oracle/static evidence 状态；
- argument shape；
- return shape；
- side effect notes；
- implementation status；
- residual unknowns。

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

当前已达到 R1 诊断启动，并完成 L2 资源依赖诊断、L3 脚本模块模型、L4 native service interface baseline、L5 service-backed runtime lifecycle、L6 oracle capture readiness、L7 title script execution checkpoint、L8 scene-entry runtime contract checkpoint、L9 scene runtime materialization checkpoint、L10 first-frame runtime contract checkpoint；这仍不是可玩的游戏执行。

C++ runtime spine 已能通过 `project.json` 加载原 sample 和 empty sample，挂载 VFS，解析 pack manifest 索引，加载 startup `.sqasm` 诊断，并通过 runtime-owned 11 个 native service interfaces 报告 native/API obligations。已验证原 sample：7 个 boot phases、2 个 loose mounts、13,028 个 pack resources、84 个 native registry APIs、11 个 native services、39 个 startup obligations，全部仍是 `not_started`。L2 已验证 title background/logo/DLC stems、title script resource refs、first-mission stage/rail-camera resources 都能通过 VFS 解析，resource report required missing 为 0。L3 已验证 title module 为 81 functions / 4,466 instructions / 593 calls / 80 closure bindings，first mission candidate 为 62 functions / 4,068 instructions / 640 calls / 61 closure bindings，且 C++ parser 可在秒级输出结构化报告。L4 已验证 11 services / 84 native APIs / 0 unowned APIs / 0 unbound APIs，但 native 行为仍没有实现。L5 已验证 original 和 empty sample 都是 `ok=true phases=7 failed_phases=0`。L6 已验证原 exe 可定位，但无 file-IO/render 捕获工具且没有用户驱动 title boot 记录，所以 P1 仍不是完成态；不要伪造 oracle。

L7 当前已有 `script-plan` 和 `script-run`：`script-plan` 解析 `setupProc -> print + modTitle.init`；`script-run --frames 1` 先加载 2 个 baseline modules：`preload.b64` 和 `script/menu/menudef.b64`，再执行 title entry，构造 `modTitle -> ModuleTitle` 和 canonical 原脚本 scene objects，执行 `setupProc`、`ModuleTitle.init` foreach scene init、首帧 `main` boot path。当前 f1 passive 指标：`constructed_objects=3`、`script_methods=86`、`script_functions=5`、`native_obligations=8`、`unique_native_apis=4`、`service_state_events=41`、`save_service_queries=4`、`platform_state_queries=3`、`ui_object_calls=16`、`unresolved_calls=0`。f1 是原脚本 boot/init 路径，不是手写菜单，也不是完整 VM。

`script-run --frames 2` 已进入真实 title scene idle：`ModuleTitle.main -> modTitle._scenes[0].main -> TitleSceneBase.main -> TitleScene.state0`。关键底层修复：`_scenes` table assignment 规范化到 `modTitle._scenes[index]`；runtime receiver 从 `_OP_PREPCALL/_OP_CALL` 传给后续分发；owner method 对 concrete object 做虚分发；script method return 会同步回传给 caller bytecode；class method lookup 优先 method binding 而不是 raw closure slot；`root.gMenu=table#1` 通过 root table alias 映射回 receiver `gMenu`。当前 f2 passive trace 指标：99 个 state-scanned functions、4,207 条 state-scanned instructions、191 个 object field writes、199 个 table slot writes、167 个 typed call returns、43 个 service state events、4 save/profile queries、3 platform queries、0 unresolved、0 truncated。`script.ScrollWindow@constructor:50._elemCount=4`，`_sel=0`，`gMenu.isDecide(false)`，因此不会再错误落入 `TitleScene.state0 -> PlaySE(3)`。

`script-run --frames 5 --input-scenario title-new-game` 已能通过 runtime input profile 驱动原始 bytecode：`TitleScene.state0 -> NewGameScene.state0 -> NewGameScene.state1 -> IsSaveFull(false) -> setMissionKey -> SetDifficultyMode -> return 200 -> ModuleTitle.fadeOut -> gMenu.startGame4Menu -> MakeNewGame(sc01) -> StartGame(mission:sc01/main/ms010_0, true)`。当前 f5 new-game 指标：`script_methods=101`、`script_functions=12`、`native_obligations=18`、`unique_native_apis=11`、`service_state_events=64`、`audio_service_commands=4`、`scene_service_commands=3`、`value_state_queries=14`、`decoded_service_arguments=21`、`unresolved_calls=0`、`truncated=false`。`StartGame` 已记录 runtime-owned save/profile/scenario state，并排队 `mission/sc01/main/ms010_0.b64.sqasm`、`map/Doujou/doujou.sge`、`map/Doujou/doujou.rcm`。这条边已由 L8 scene-entry contract 承接到 first mission `setupProcess`。

`script-run samples/touhou_new_world/project.json mission/sc01/main/ms010_0.b64 setupProcess --frames 1` 已执行 first mission 原始 `setupProcess`，覆盖 `Loader -> LoadStage -> LoadEventsScriptViaMission -> ClearCurrentQuest -> CallSetupEvents -> GetMarkerFromRequest -> PushPlayerChara(reimuEx) -> PushTaskGameCamera -> SetCheckPoint -> LoadRailCamera -> SetEnableRailCamera -> SetEnableAutoCameraAdjust -> SetDefaultCameraState -> LoadCharaPlace -> FadeOutBGM -> GetPlaceParams -> loader.end`。当前 f1 first-mission 指标：`native_obligations=18`、`unique_native_apis=18`、`engine_object_calls=9`、`typed_call_returns=35`、`service_state_events=27`、`scene_service_commands=10`、`audio_service_commands=1`、`decoded_service_arguments=19`、`unresolved_calls=0`、`truncated=false`。runtime service state 已记录 stage `map/Doujou/doujou.sge`、event script `sc01/main/ms010_0`、player `reimuEx`、rail camera `map/Doujou/doujou.rcm`、checkpoint marker 和 place params。

`yuengine_cli scene-entry samples/touhou_new_world/project.json --repo-root .` 已把 title new-game 服务状态和 first mission setup 服务状态合并成正式 runtime contract。当前 scene-entry 指标：`ok=true`、`title_new_game_executed=true`、`mission_setup_executed=true`、`stage_ready=true`、`actor_ready=true`、`camera_ready=true`、`event_ready=true`、`resource_bindings=6`、`missing_resources=0`、`script_bindings=5`、`missing_scripts=0`。已绑定 `map/doujou/doujou.sge`、`map/doujou/doujou.mdl`、`map/doujou/doujou.col`、`map/doujou/doujou.rcm`、`player/reimuex.b64`、`player/reimuex_pcg.b64`、mission/event/player script modules。当前边界转为解析和物化 SGE/RCM/COL/MDL、event marker、actor task、camera task、renderer/audio/input runtime handles，而不是停在 path binding。

`yuengine_cli scene-runtime samples/touhou_new_world/project.json --repo-root .` 已消费 L8 scene-entry 并读取真实资源 payload。当前 scene-runtime 指标：`ok=true`、`scene_entry_ok=true`、`stage_handle_ready=true`、`actor_handle_ready=true`、`camera_handle_ready=true`、`event_marker_ready=true`、`stage_dependencies=42`、`missing_stage_dependencies=0`、`model_meshes=111`、`collision_triangles=150`、`rail_nodes=3`。已确认 `MgResourceHeader` 的 SGE/MDL/COL/RCM payload、道场 42 个依赖、39 个 DDS 纹理、272 个碰撞顶点、450 个索引、3 个 rail node 候选、player b64/pcg payload、event marker 表达式。当前边界转为 renderer/audio/input/gameplay frame contract。

`yuengine_cli first-frame samples/touhou_new_world/project.json --repo-root .` 已消费 L9 scene-runtime handles 并产出 renderer/input/event 首帧 runtime contract。当前 first-frame 指标：`ok=true`、`scene_runtime_ok=true`、`renderer_frame_ready=true`、`actor_frame_ready=true`、`camera_frame_ready=true`、`input_frame_ready=true`、`event_frame_ready=true`、`mesh_draw_candidates=111`、`texture_bindings=39`、`collision_triangles=150`、`actor_instances=1`、`rail_nodes=3`、`event_markers=1`。renderer profile 为 `d3d9_compatible`，输入 ownership 为 `Actor And Task Service` / `player_actor_camera_scene`。当前边界转为 first mission event thread/tutorial/player-control 行为和真实 renderer command-buffer/upload 层。

重要语义修复：`_OP_NEWSLOT/_OP_NEWSLOTA` 是 slot write，不得覆盖 register `a0`，否则 root table 会被污染并丢失 `ModuleBase`。root slot 写入会把 `modTitle = ModuleTitle()` 规范化到 canonical `modTitle` 对象，基类/super 方法执行也不会覆盖对象的 concrete class。P4 仍不是完成态，因为 confirmed native、argument/return shape、side effects、oracle/static evidence 和 implementation status 还没有逐行确认；P6 现在已有 C++ service interface baseline、首批 runtime-owned service state、首批 runtime-owned script/object state，但还不是完成态，因为完整 API behavior implementation、typed argument/return contracts、`gMenu` 可配置输入、save/new-game 分支和 UI command payload 尚未落地；R2 也不是完成态，因为当前只是 title boot/title idle bytecode checkpoint，不是完整 Squirrel VM，也没有真实 title UI/gameplay。

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

当前下一步见 `docs/LOOP_TASKS.md`，优先 L11 边界。当前 L7 已有 multi-module title boot/title idle/new-game bytecode/service/script-object state checkpoint；L8 已执行 `mission/sc01/main/ms010_0.b64.sqasm` 的 `setupProcess` 并通过 `scene-entry` 产出 binding contract；L9 已把这些 binding 物化成 stage graph、event marker、actor task、camera task payload handles；L10 已把 handles 接到 renderer/input/event first-frame contract。下一条边是执行或建模 first mission event thread/tutorial/player-control 行为，同时继续扩展 Continue/Load/Option/Exit 输入场景。

Runtime implementation 已开始，但只能按完整引擎主干推进，不能写临时视觉 demo。所有未确认 native 行为必须进入 obligation/diagnostics。

当前 Ninja/MSVC 构建的 `/showIncludes` dependency prefix 是乱码。改 C++ header 后必须用 `cmake --build build\cmake-bt143 --clean-first` 或新 build 目录验证，否则可能复用旧对象导致 ABI 崩溃。

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
- 多 agent 第一轮不要同时修改 `AGENTS.md`。除非任务就是维护接力规范，否则只能读它。
- 大型 generated artifacts 不进入默认分支。并行 agent 需要共享时，写生成命令和相对路径，不直接提交大文件。

## Git Rules

- `YuEngine` 是正式 repo。
- 改动前后检查 `git status`。
- 不要在外层目录留下正式工程代码。
- 不要自动 push。需要 push 时明确告诉用户。
- 不要 reset、checkout、clean 用户改动。
- 如果遇到不属于自己的改动，保留并绕开；如果影响当前任务，先说明冲突。
- 不要把未审计的 `Project2` 批量迁移结果直接提交。
- 不要提交 10MB 以上的 generated artifact，除非用户明确要求并说明原因。

## Engineering Rules

通用规则已经从用户的 `AGENTS.md` 和 `skills-main` 提炼过：

- 商业项目质量：性能、维护性、拓展性、稳定性同时考虑。
- 先证据，后实现。
- 先建立反馈回路，再调试或实现。
- 修 bug 走 reproduce -> minimize -> hypotheses -> instrument -> fix -> regression。
- 测试行为，不测内部形状。
- 完整层级闭环，不做横向堆文件，也不做临时壳式最小入口。
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
```

如果 sibling `..\Project2` 不可访问，先不要继续猜。需要把历史准备材料导入 `YuEngine`。

不是第一轮校准命令，但后续 lane 可以使用：

```powershell
python ..\Project2\tools\entrypoint_closure.py script/menu/titlemenu.b64.sqasm
```

运行这类命令前，必须说明会写哪些输出；如果只是探索，优先写到临时目录或当前 lane 的明确 output/cache 目录。

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
