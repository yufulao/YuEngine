// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetLoadState.h

#pragma once

namespace yuengine::asset {
enum class AssetLoadState {
    Unloaded,
    Loading,
    Decoded,
    Uploaded,
    Resident,
    Failed
};
}
