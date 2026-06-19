// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsCounters.h

#pragma once

#include <cstdint>

namespace yuengine::diagnostics {
struct RuntimeDiagnosticsCounters final {
    std::uint64_t frame_count = 0U;
    std::uint64_t frame_time_nanoseconds = 0U;
    std::uint64_t object_count = 0U;
    std::uint64_t resource_count = 0U;
    std::uint64_t render_submission_count = 0U;
    std::uint64_t audio_submission_count = 0U;
    std::uint64_t input_command_count = 0U;
};
}
