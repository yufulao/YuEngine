# ThirdParty

本目录预留给经过审查、明确需要随仓库分发的第三方包。

`Samples/AssetSmokeDemo` 不在这里 vendored Ogg 或 Vorbis 二进制。它默认检查同级 `ue\Engine` 目录作为 Unreal Engine 依赖来源，并在示例构建阶段把需要的运行时 DLL 复制到 ignored 的构建目录中；需要时可通过 `UE_ENGINE_ROOT` 或 `YU_ASSET_SMOKE_UE_ENGINE_ROOT` 覆盖。若本机只有 UE 源码而缺少 import library 或 runtime DLL，示例必须报告显式依赖缺口。

不要把生成出来的 DLL 副本、本地 SDK 缓存目录或构建输出提交到 `ThirdParty`。
