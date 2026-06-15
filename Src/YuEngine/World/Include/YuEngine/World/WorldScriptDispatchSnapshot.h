// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/World/WorldScriptDispatchStatus.h"

namespace yuengine::world {
struct WorldScriptDispatchSnapshot final {
    std::uint32_t binding_capacity = 0U;
    std::uint32_t binding_count = 0U;
    std::uint64_t dispatched_call_count = 0U;
    std::uint64_t skipped_phase_count = 0U;
    std::uint64_t failed_dispatch_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::script::ScriptStatus last_script_status = yuengine::script::ScriptStatus::Success;
    WorldScriptDispatchStatus last_status = WorldScriptDispatchStatus::Success;
};
}
