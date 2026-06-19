// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookStatus.h

#pragma once

namespace yuengine::diagnostics {
enum class RuntimeDiagnosticsOverlayHookStatus {
    Success,
    Disabled,
    InvalidArgument,
    CapacityExceeded,
    RuntimeDependencyRejected
};
}
