// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookStatus.h"

namespace yuengine::diagnostics {
struct RuntimeDiagnosticsOverlayHookResult final {
    RuntimeDiagnosticsOverlayHookStatus status = RuntimeDiagnosticsOverlayHookStatus::Success;
    bool is_optional_tooling_plane = true;
    bool runtime_dependency_required = false;
    std::uint32_t accepted_line_capacity = 0U;
    std::uint32_t observed_counter_count = 0U;
};
}
