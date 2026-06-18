# YuAssetSmokeDemo

`YuAssetSmokeDemo` 是贴近引擎的独立 smoke 示例。它通过 D3D11 渲染带贴图网格，并通过 Ogg/Vorbis 解码仓库内的示例音乐，再交给 XAudio2 循环播放。

这个示例不是 YuEngine 运行时代码，也不是整个硬件层已经完整落地的证明。它是放在 `Src/YuEngine` 之外的独立证明包。

## 目录

- `Source/Main.cpp`：独立 Win32/D3D11/XAudio2 示例程序。
- `Assets/`：smoke 运行使用的仓库内示例输入。
- `CMakeLists.txt`：示例自己的独立构建文件。
- `RunAssetSmokeDemo.ps1`：配置、构建和运行辅助脚本。

`Build/`、生成截图、日志和复制出来的运行时 DLL 都是生成物，必须保持 ignored。

## 第三方依赖

本示例不在仓库中 vendored Ogg/Vorbis 二进制。CMake 期望本机 Unreal Engine 安装目录提供 Ogg 和 Vorbis 的头文件、导入库和运行时 DLL：

- `Source/ThirdParty/Ogg/libogg-1.2.2`
- `Source/ThirdParty/Vorbis/libvorbis-1.3.2`
- `Binaries/ThirdParty/Ogg/Win64/VS2015`
- `Binaries/ThirdParty/Vorbis/Win64/VS2015`

调用方可以设置 `UE_ENGINE_ROOT` 环境变量，或设置 CMake cache 变量 `YU_ASSET_SMOKE_UE_ENGINE_ROOT`。辅助脚本会在 `UE_ENGINE_ROOT` 存在时转发给 CMake。

仓库内的音乐、网格、贴图和材质文件是示例输入，来源记录为提交 `a3c7ede` 引入的现有样例资源。本次清理不新增第三方资源，也不扩展这些资源的许可证声明。

## 运行

在本目录执行：

```powershell
$env:UE_ENGINE_ROOT = 'D:\app\Epic Games\UE_5.5\Engine'
powershell -ExecutionPolicy Bypass -File .\RunAssetSmokeDemo.ps1
```

期望输出：

```text
YuAssetSmokeDemo PASS mesh_vertices=36 texture=128x128 bgm=XAudio2/Vorbis seconds=8
```

该示例仍然不依赖 YuEngine 构建输出，但需要 Windows D3D11/XAudio2，以及上文说明的本机 Ogg/Vorbis 依赖。
