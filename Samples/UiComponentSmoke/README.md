# UiComponentSmoke

`UiComponentSmoke` 是 UI Stage 2 的组件库 smoke sample。它读取一个仓库内 `YuUILayout` 示例文件，在同一个窗口中运行 `UiCore` 的 Text、Image、Button、Slider 和 GridView/List virtualization value path，并用进程退出码和 `PASS` 输出报告结果。

本示例只覆盖 Stage 2 component library smoke sample，不实现后续性能统计、验证文档、编辑器或项目迁移范围，也不进入渲染设备路径。

## 目录

- `Layouts/ComponentWindow.YuUILayout.json`：组件库 smoke layout 输入。
- `Source/UiComponentSmokeSample.*`：可复用的 sample 核心路径。
- `Source/Main.cpp`：命令行 smoke sample 入口。

## 运行

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuUiComponentSmokeSample YuUiComponentSmokeSampleTests -- /v:minimal
ctest --preset windows-fast-gate -R "^UiComponentSmokeSample_" --output-on-failure
.\build\windows-fast-gate-vs\Debug\YuUiComponentSmokeSample.exe --layout Samples\UiComponentSmoke\Layouts\ComponentWindow.YuUILayout.json
```

期望 sample 输出：

```text
YuUiComponentSmokeSample PASS nodes=11 text=4 image=1 button=900 slider=0.25 gridVisible=10 gridPool=20 gridDirty=3
```
