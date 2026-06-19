// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Src/RuntimeDiagnosticsOverlayHook.cpp

#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHook.h"

namespace yuengine::diagnostics {
RuntimeDiagnosticsOverlayHookResult RuntimeDiagnosticsOverlayHook::ValidateProposal(
    const RuntimeDiagnosticsOverlayHookProposal &proposal) const {
    RuntimeDiagnosticsOverlayHookResult result{};
    result.is_optional_tooling_plane = true;
    result.runtime_dependency_required = proposal.requires_runtime_dependency;

    if (!proposal.is_enabled) {
        result.status = RuntimeDiagnosticsOverlayHookStatus::Disabled;
        return result;
    }

    if (proposal.requires_runtime_dependency) {
        result.status = RuntimeDiagnosticsOverlayHookStatus::RuntimeDependencyRejected;
        return result;
    }

    if (proposal.requested_line_capacity == 0U) {
        result.status = RuntimeDiagnosticsOverlayHookStatus::InvalidArgument;
        return result;
    }

    if (proposal.requested_line_capacity > MAX_RUNTIME_DIAGNOSTICS_OVERLAY_LINES) {
        result.status = RuntimeDiagnosticsOverlayHookStatus::CapacityExceeded;
        return result;
    }

    result.status = RuntimeDiagnosticsOverlayHookStatus::Success;
    result.accepted_line_capacity = proposal.requested_line_capacity;
    result.observed_counter_count = proposal.observed_counter_count;
    return result;
}
}
