// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRuntimeFixtureStatus.h

#pragma once

namespace yuengine::asset {
enum class AssetRuntimeFixtureStatus {
    Success,
    InvalidArgument,
    DependencyTraversalFailed,
    AssetLoadingFailed,
    AssetDecodedFailed,
    TextureReadyFailed,
    AudioReadyFailed,
    ResourceRefreshFailed,
    AssetQueryFailed
};
}
