# UiCoreSmoke

`UiCoreSmoke` 是 UI Stage 1 的最小 smoke sample。它读取一个仓库内 `YuUILayout` 示例文件，构建 `UiCore` node tree，运行 layout pass 和 draw-list builder，再通过 `YuUiRenderCoreBridge` 提交到 `RenderCore` fixture / `NullRhiDevice`，最后用进程退出码和 `PASS` 输出报告结果。

本示例只覆盖 `UI-S1-008` 的 Stage 1 运行证明，不实现 Stage2 Text/Image/Button/Slider/GridView，不引入 Project UI Runtime lifecycle/config/window stack，不使用 editor preview，也不导入 Dear ImGui。

## 目录

- `Layouts/SimpleWindow.YuUILayout.json`：最小 UI layout 输入。
- `Source/UiCoreSmokeSample.*`：可复用的 sample 核心路径。
- `Source/Main.cpp`：命令行 smoke sample 入口。

## 运行

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuUiCoreSmokeSample YuUiCoreSmokeSampleTests -- /v:minimal
ctest --preset windows-fast-gate -R "^UiCoreSmokeSample_" --output-on-failure
.\build\windows-fast-gate-vs\Debug\YuUiCoreSmokeSample.exe --layout Samples\UiCoreSmoke\Layouts\SimpleWindow.YuUILayout.json
```

期望 sample 输出：

```text
YuUiCoreSmokeSample PASS nodes=3 containers=2 draws=2 submitted=2 renders=2
```
