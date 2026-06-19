// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookProposal.h

#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
inline constexpr std::uint32_t MAX_RUNTIME_DIAGNOSTICS_OVERLAY_LINES = 8U;

struct RuntimeDiagnosticsOverlayHookProposal final {
    bool is_enabled = false;
    bool requires_runtime_dependency = false;
    std::uint32_t requested_line_capacity = 0U;
    std::uint32_t observed_counter_count = 0U;
};
}
