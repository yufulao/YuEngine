// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffStatus.h

#pragma once

namespace yuengine::runtimeassetworldadapter {
enum class RuntimeAssetWorldObjectRestoreHandoffStatus {
    Success,
    InvalidAdapterRequest,
    AdapterBuildFailed,
    GateFailed,
    RestoreFailed
};
}
