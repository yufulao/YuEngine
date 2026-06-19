// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/RuntimeConfigRecord.h

#pragma once

#include <cstdint>

namespace yuengine::serialize {
constexpr std::uint32_t RUNTIME_CONFIG_SCHEMA_VERSION = 1U;

struct RuntimeConfigRecord final {
    std::uint32_t schema_version = RUNTIME_CONFIG_SCHEMA_VERSION;
    std::uint32_t fixed_step_microseconds = 0U;
    std::uint32_t max_frame_count = 0U;
    std::uint32_t command_snapshot_capacity = 0U;
    bool diagnostics_enabled = false;
};
}
