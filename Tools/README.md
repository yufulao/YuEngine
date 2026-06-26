# Tools

本目录预留给仓库工具和开发辅助程序。

不属于运行时模块、也不属于示例的工具源码应放在这里。生成输出、下载的 SDK、截图和本地构建目录应放在所属工具附近的 ignored 路径中。

`ENG-161C` 没有新增工具包。

## Focused test helper

`RunFocusedTests.ps1` is the direct-main focused-first CTest helper. It lists
tests by default and only runs a focused subset when `-Action Run` is passed.

Examples:

```powershell
.\Tools\RunFocusedTests.ps1 -Test RuntimeAssetData_FormatHeaderRejectsUnsupportedVersion
.\Tools\RunFocusedTests.ps1 -Action Run -Module RuntimeAssetData
.\Tools\RunFocusedTests.ps1 -ChangedPath Src\YuEngine\RuntimeAsset\Src\RuntimeAssetData.cpp
.\Tools\RunFocusedTests.ps1 -Action Run -Build -BuildTarget YuRuntimeAssetDataClosedLoopTests -Test RuntimeAssetData_FormatHeaderRejectsUnsupportedVersion
```

Prefer `-Test` for exact named tests and `-ChangedPath` or `-Module` for a
small module label surface. Do not use this helper as a full CTest replacement;
full gates remain explicit review or release decisions.
