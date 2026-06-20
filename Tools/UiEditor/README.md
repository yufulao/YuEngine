# YuUiEditor

`Tools/UiEditor` 是 editor-only 工具目录，用于承载 `UI-E1-001` 的 editor shell 首片。这里的代码不能被 runtime UI Core 依赖，也不能把 Dear ImGui、D3D11、RenderCore preview viewport 或 Project UI Runtime lifecycle/config/window stack 传播到 `Src/YuEngine/UiCore`。

## Dear ImGui 依赖状态

`ENG-183C` 对当前仓库做了依赖核对：

- `ThirdParty/` 只保留经过审查、明确需要随仓库分发的第三方包，目前没有 Dear ImGui。
- 根 `CMakeLists.txt` 和源码中没有 Dear ImGui、ImGui 或等价 editor shell 依赖配置。
- 因此当前不能声明真实 Dear ImGui docking shell 已落地。

后续如果要接入真实 Dear ImGui 后端，必须先补齐：

- 第三方来源、版本、许可证和仓库内分发策略。
- `ThirdParty/DearImGui` 或显式本地依赖入口的 CMake gate。
- editor-only 目标链接边界，确保 runtime library 不 include 或 link editor shell。

## 当前首片

当前落地的是可构建、可测试的 shell state 首片：

- `YuUiEditorShellCore`：维护 hierarchy、inspector、preview 三个 placeholder panel 的 editor-only registry。
- `YuUiEditorShell`：构建一个 shell entry；在 Dear ImGui 后端未配置时返回 explicit unavailable，不伪装成真实 docking UI。
- `YuUiEditorTests`：headless 验证默认 placeholder、panel 开关、输出容量和 Dear ImGui backend gate。

## Layout Asset Load

`ENG-184D` 将 `UI-E1-002` 收敛为 editor-only headless 首片：

- `YuUiEditorShellCore` 可加载 `YuEngine.UI.Layout` / `YuUILayout` 文本中的 `schema`、`version`、`layoutId`、`rootNodeId` 和 `nodes`。
- hierarchy panel 导出 stable node id、parent id、order、name 和 type。
- inspector record 跟随当前选中 node，默认选中 root node。
- duplicate node id、missing parent、missing root 和输出容量不足会返回显式状态。

当前 loader 只服务 editor 检查面，不做 runtime preview，不运行 Project UI Runtime lifecycle，也不引入 Dear ImGui 或 RenderCore 依赖。

## Runtime Preview Viewport

`ENG-185B` 将 `UI-E1-003` 收敛为 editor-only headless 首片：

- `YuUiEditorShellCore` 在 layout asset 已加载后，可显式构建 runtime preview viewport。
- preview route 使用 `UiNodeTree`、`UiLayoutPass`、`UiDrawListBuilder`、`UiRenderCoreBridge`、`RenderFixturePass` 和 `NullRhiDevice`，属于无窗口 headless RenderCore 等价路径。
- preview panel 只有在 headless RenderCore route 成功提交后才从 placeholder 切换为 data-backed。
- Dear ImGui visual docking backend 仍然保持 explicit unavailable；本首片不静默引入 Dear ImGui，不声明真实 docking viewport 已完成。

当前 preview viewport 只覆盖 layout-to-draw-to-RenderCore fixture 的确定性首片，不运行 Project UI Runtime lifecycle，不接入 D3D11/真实 GPU backend，也不扩展 UI-E2/E3。

## ID/Event Validator

`ENG-207A` 将 `UI-E1-006` 收敛为 editor-only validation 首片：

- `UiEditorIdEventValidator` 独立于 `UiEditorShellState`，输入由 editor layout 抽取的 node id 与 event binding。
- duplicate node id 和 missing event name 会输出带 `issue_kind`、`node_id`、`context_id`、计数和状态的 validation report。
- validator 先统计再写入 report buffer；输出容量不足、invalid node 或 invalid event 时不会污染调用方 report buffer。

当前 validator 只证明 E1 的 ID/event 检查面，不接入真实 Dear ImGui 后端，不扩展 UI-E2/E3，也不改变 runtime UiCore ownership/lifecycle。

## Component Templates

`ENG-208A` 将 `UI-E2-001` 收敛为 editor-only component template 首片：

- `UiEditorComponentTemplateFactory` 可为 Text、Image、Button、Slider、GridView 创建确定性的 layout node record。
- template 输出包含 `kind`、`default_state`、layout node、resource references 和 event bindings。
- 输出可被现有 layout asset loader、`UiEditorResourceReferenceValidator` 和 `UiEditorIdEventValidator` 验证。
- buffer 容量不足、invalid template kind 或 invalid layout node 会返回显式状态，并避免污染调用方输出。

当前 component template 首片不做 state preview、GridView 数据模拟、性能诊断、真实 Dear ImGui backend、RHI/D3D11/RenderCore backend 扩展，也不改变 Project UI Runtime 或产品窗口迁移边界。

## State Preview

`ENG-209A` 将 `UI-E2-002` 收敛为 editor-only Button state preview 首片：

- `UiEditorStatePreviewFactory` 接收 `UiEditorComponentTemplateFactory` 创建的 Button template record。
- preview 输出 normal、hover、pressed、disabled、selected 的可见状态名和显式标记。
- disabled state 输出 `disabled_overlay_visible`，selected state 输出 `selected_outline_visible`。
- 非 Button template、invalid state 和 invalid output 都返回显式状态，并避免污染调用方 preview record。

当前 state preview 首片不做 GridView 数据模拟、性能诊断、真实 Dear ImGui backend、RHI/D3D11/RenderCore backend 扩展，也不改变 UiCore lifecycle、Project UI Runtime 或产品窗口迁移边界。

## GridView Data Simulation

`ENG-210A` 将 `UI-E2-003` 收敛为 editor-only GridView data simulation 首片：

- `UiEditorGridViewDataSimulationFactory` 接收 `UiEditorComponentTemplateFactory` 创建的 GridView template record。
- simulation 输入 `sample_count`、`first_visible_item_index`、`visible_cell_count` 和 `buffer_cell_count`，输出确定性的 visible/buffer cell preview。
- 输出记录包含 sample count、实际写入 cell 数、visible cell 数、buffer cell 数和最后一个 visible item index。
- invalid template、invalid sample/count、buffer 上限和输出容量不足会返回显式状态，并避免污染调用方 record/cell buffer。

当前 GridView data simulation 首片只证明 editor preview 的 sample count 到 visible/buffer cell 映射，不做 UI-E3 性能诊断、anti-pattern warnings、真实 Dear ImGui backend、RHI/D3D11/RenderCore backend 扩展，也不改变 UiCore lifecycle、Project UI Runtime 或产品窗口迁移边界。
