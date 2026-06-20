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
