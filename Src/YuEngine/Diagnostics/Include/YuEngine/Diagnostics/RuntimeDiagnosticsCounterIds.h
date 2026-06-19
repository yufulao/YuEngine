// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsCounterIds.h

#pragma once

#include <cstddef>

#include "YuEngine/Diagnostics/DiagnosticsCounterId.h"

namespace yuengine::diagnostics {
inline constexpr std::size_t RUNTIME_DIAGNOSTICS_COUNTER_COUNT = 7U;
inline constexpr DiagnosticsCounterId RUNTIME_FRAME_COUNT_COUNTER_ID{1001U};
inline constexpr DiagnosticsCounterId RUNTIME_FRAME_TIME_COUNTER_ID{1002U};
inline constexpr DiagnosticsCounterId RUNTIME_OBJECT_COUNT_COUNTER_ID{1003U};
inline constexpr DiagnosticsCounterId RUNTIME_RESOURCE_COUNT_COUNTER_ID{1004U};
inline constexpr DiagnosticsCounterId RUNTIME_RENDER_COUNT_COUNTER_ID{1005U};
inline constexpr DiagnosticsCounterId RUNTIME_AUDIO_COUNT_COUNTER_ID{1006U};
inline constexpr DiagnosticsCounterId RUNTIME_INPUT_COUNT_COUNTER_ID{1007U};
}
