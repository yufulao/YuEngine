// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsRecordResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"
#include "YuEngine/Diagnostics/DiagnosticsStatus.h"

namespace yuengine::diagnostics {
struct RuntimeDiagnosticsRecordResult final {
    DiagnosticsStatus status = DiagnosticsStatus::Success;
    std::uint32_t contract_counter_count = 0U;
    std::uint32_t recorded_counter_count = 0U;
    DiagnosticsCounterId failed_counter_id{};
};
}
