# ThirdParty

本目录预留给经过审查、明确需要随仓库分发的第三方包。

`Samples/AssetSmokeDemo` 不在这里 vendored Ogg 或 Vorbis 二进制。它使用本机 Unreal Engine 安装目录作为依赖来源，并在示例构建阶段把需要的运行时 DLL 复制到 ignored 的构建目录中。

不要把生成出来的 DLL 副本、本地 SDK 缓存目录或构建输出提交到 `ThirdParty`。
