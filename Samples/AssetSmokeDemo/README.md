# YuAssetSmokeDemo

`YuAssetSmokeDemo` 是贴近引擎的 L0 纵向示例。它通过示例自己的 D3D11 路径渲染带贴图网格，并通过 Ogg/Vorbis 解码仓库内的示例音乐，再交给 XAudio2 循环播放；默认构建还会拉起 YuEngine L0 证据路径，覆盖文件读取、Resource 解码载荷、Streaming 到 RHI 贴图上传、RenderCore view/draw/material 值路径、HardwareFrameHost 窗口帧、输入轮询、音频设备状态、resize 和 shutdown。

这个示例不是 gameplay/UI，也不声明 L1 以上系统完成。它是放在 `Src/YuEngine` 之外的 L0 运行时纵向证明包。

## 目录

- `Source/Main.cpp`：独立 Win32/D3D11/XAudio2 示例程序入口。
- `Source/L0EngineEvidence.*`：调用 YuEngine L0 模块的纵向证据路径。
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

## YuEngine 证据路径

默认启用 `YU_ASSET_SMOKE_ENGINE_EVIDENCE=ON`。CMake 会从 `YU_ASSET_SMOKE_ENGINE_ROOT` 指向的仓库根目录构建最小 YuEngine 依赖，默认值是示例目录上两级的当前仓库根目录。

该路径只用于 L0 纵向证明，不复制或依赖仓库里预提交的可执行文件。它会将完整示例纹理裁剪成 2x2 decoded payload，保持在当前 Resource 记录大小约束内，再走 Resource/Streaming/RHI/RenderCore/HardwareFrameHost 的真实模块接口。

`gamepad=graded_skip` 表示当前机器没有可用手柄或 XInput 源不可用；`audio=graded_skip` 表示当前机器没有可用 XAudio2 输出设备或没有拿到硬件 completion。二者都是显式的硬件环境状态，不会伪装成 pass。

## 运行

在本目录执行：

```powershell
$env:UE_ENGINE_ROOT = 'D:\app\Epic Games\UE_5.5\Engine'
powershell -ExecutionPolicy Bypass -File .\RunAssetSmokeDemo.ps1
```

期望输出：

```text
YuAssetSmokeDemo PASS mesh_vertices=36 texture=128x128 bgm=XAudio2/Vorbis seconds=8
YuAssetSmokeDemo L0_ENGINE PASS file_bytes=... decoded_texture=2x2 upload_generation=... render_frames=... input_events=... gamepad=pass|graded_skip audio=pass|graded_skip resize=pass shutdown=pass
```

该示例不依赖预提交的 YuEngine 构建输出，但默认会从源码构建必要的 YuEngine L0 模块。运行需要 Windows D3D11/XAudio2，以及上文说明的本机 Ogg/Vorbis 依赖。
