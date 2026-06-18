// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptStatus.h"

namespace yuengine::script {
struct ScriptSnapshot final {
    std::uint32_t binding_capacity = 0U;
    std::uint32_t binding_count = 0U;
    std::uint64_t successful_call_count = 0U;
    std::uint64_t failed_call_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    ScriptStatus last_status = ScriptStatus::Success;
};
}
